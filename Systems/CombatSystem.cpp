#include "CombatSystem.hpp"

void CombatSystem::init() {
	this->vault->dispatcher.connect<AnimationFrameChanged>(this);
}

// frame changed
void CombatSystem::receive(const AnimationFrameChanged &event) {
	if (event.state == "attack") {
		EntityID entity = event.entity;
		int frame = event.frame;
		if (frame == 1) {
			Tile &tile = vault->registry.get<Tile>(entity);
			Unit &unit = vault->registry.get<Unit>(entity);

#ifdef COMBAT_DEBUG
			std::cout << "CombatSystem: play sound " << unit.attackSound << std::endl;
#endif
			map->sounds.push(SoundPlay {unit.attackSound, 1, false, tile.pos});

			if (unit.targetEnt) {
				if (this->vault->registry.valid(unit.targetEnt)) { // ???
					Tile &destTile = this->vault->registry.get<Tile>(unit.targetEnt);
					sf::Vector2f fxPos = destTile.ppos;
					sf::Vector2i diffPos = tile.pos - destTile.pos;
					fxPos.x += diffPos.x * 8.0 + 16.0;
					fxPos.y += diffPos.y * 8.0;

					ParticleEffectOptions hitOptions;
					hitOptions.destPos = destTile.ppos;
					hitOptions.direction = getDirection(tile.pos, destTile.pos);
					hitOptions.screenSize = sf::Vector2i(this->screenWidth, this->screenHeight);

					this->emitEffect("hit", unit.targetEnt, fxPos, hitOptions);

					ParticleEffectOptions projOptions;
					projOptions.destPos = destTile.ppos;
					projOptions.direction = getDirection(tile.pos, destTile.pos);
					projOptions.screenSize = sf::Vector2i(this->screenWidth, this->screenHeight);

					EntityID projEnt = this->emitEffect("projectile", entity, tile.ppos, projOptions);
					if (projEnt && unit.canDestroyResources) {
						ParticleEffect &proj = this->vault->registry.get<ParticleEffect>(projEnt);
						sf::Vector2i projDestPos = destTile.pos;
						proj.effectEndCallback = [this, projDestPos]() {
							EntityID resEnt = this->map->resources.get(projDestPos.x, projDestPos.y);
							if (resEnt) {
								this->map->resources.set(projDestPos.x, projDestPos.y, 0);
								this->vault->registry.destroy(resEnt);
								std::cout << "DESTROY RESOURCE AT " << projDestPos.x << "x" << projDestPos.y << std::endl;
							}
						};
					}

				} else {
					this->changeState(entity, "idle");
					unit.targetEnt = 0;
					unit.destpos = tile.pos;
				}
			}
		}
	}
}

void CombatSystem::updateFront(float dt) {
	// player
	auto playerView = this->vault->registry.view<Player>();
	for (EntityID entity : playerView) {
		Player &player = playerView.get(entity);

		std::vector<Point> points;
		for (auto &p : player.allFrontPoints) {
			points.push_back(Point{(double)p.x, (double)p.y});
		}
		player.allFrontPoints.clear();
		player.frontPoints.clear();

		std::vector<int> labels;

		int num = dbscan(points, labels, 10.0, 3);

//			std::cout << player.team << " cluster size is " << num << "/" << points.size() << std::endl;

		std::map<int, Point> points_map;
		std::map<int, int> points_map_size;
		for (int i = 0; i < (int)points.size(); i++) {
			if (points_map.count(labels[i]) == 0) {
				points_map[labels[i]] = points[i];
				points_map_size[labels[i]] = 1;
			} else {
				points_map[labels[i]].x = points_map[labels[i]].x + points[i].x;
				points_map[labels[i]].y = points_map[labels[i]].y + points[i].y;
				points_map_size[labels[i]] = points_map_size[labels[i]] + 1;
			}
//				std::cout << "Point(" << points[i].x << ", " << points[i].y << "): " << labels[i] << std::endl;
		}

		for (auto pair : points_map) {
			player.frontPoints.push_back(FrontPoint{sf::Vector2i(pair.second.x / points_map_size[pair.first], pair.second.y / points_map_size[pair.first]), points_map_size[pair.first]});
		}

		std::sort (player.frontPoints.begin(), player.frontPoints.end(), FrontPointCompare());
//			for (FrontPoint &p : player.frontPoints) {
//				std::cout << player.team << " front point " << p.pos.x << "x" << p.pos.y << " " << p.priority << std::endl;
//			}
	}
}

bool CombatSystem::posInRange(Tile &tile, sf::Vector2f &destPos, int dist, int maxDist) {
	return (distance(tile.ppos, destPos) >= (dist - 1) * RANGE_RADIUS && distance(tile.ppos, destPos) <= (maxDist) * RANGE_RADIUS);
	// || distance(tile.ppos, destPos) <= RANGE_RADIUS;
}

bool CombatSystem::ennemyInRange(Tile &tile, Tile &destTile, int dist, int maxDist) {
	for (int w = 0; w < destTile.size.x; ++w) {
		for (int h = 0; h < destTile.size.y; ++h) {
			sf::Vector2f p = destTile.ppos + sf::Vector2f((w - destTile.size.x / 2) * 32, (h - destTile.size.x / 2) * 32);
			if (this->posInRange(tile, p, dist, maxDist)) {
				return true;
			}
		}
	}
	return false;
}

void CombatSystem::update(float dt) {

	// pass 1, if an ennemy is in sight, then attack / respond to attack
	auto view = this->vault->registry.persistent<Tile, GameObject, Unit>();
	// respond to attack
	for (EntityID entity : view) {
		Tile &tile = view.get<Tile>(entity);
		GameObject &obj = view.get<GameObject>(entity);
		Unit &unit = view.get<Unit>(entity);

		if (unit.targetEnt && this->vault->registry.valid(unit.targetEnt)) {
			if (this->vault->registry.has<Unit>(unit.targetEnt)) {
				Tile &destTile = this->vault->registry.get<Tile>(unit.targetEnt);
				GameObject &destObj = this->vault->registry.get<GameObject>(unit.targetEnt);
				Unit &destUnit = this->vault->registry.get<Unit>(unit.targetEnt);

				if (destObj.life > 0) {
					if (this->vault->registry.has<Unit>(unit.targetEnt)) {
						Unit &destUnit = this->vault->registry.get<Unit>(unit.targetEnt);
						if (destTile.state == "idle") {
							// if ennemy is idle, he will fight back
							this->attack(destUnit, entity);
							destUnit.destpos = destTile.pos;
						} else if (destTile.state == "move") {
//								this->stop(destUnit);
//								unit.destpos = tile.pos;
							this->attack(destUnit, entity);
							destUnit.destpos = destTile.pos;
						} else if (destTile.state == "attack" && destUnit.targetEnt) {
							// if ennemy is attacking a building, he will fight back
							if (this->vault->registry.valid(destUnit.targetEnt) && this->vault->registry.has<Building>(destUnit.targetEnt)) {
								this->attack(destUnit, entity);
								destUnit.destpos = destTile.pos;
							}
						}
					}
				}
			}
		}
	}

	// attack in sight
	for (EntityID entity : view) {
		Tile &tile = view.get<Tile>(entity);
		GameObject &obj = view.get<GameObject>(entity);
		Unit &unit = view.get<Unit>(entity);
		if (!unit.targetEnt) {
			EntityID finalTargetEnt = 0;
			float dist = std::numeric_limits<float>::max();

			for (sf::Vector2i const &p : this->tileSurfaceExtended(tile, obj.view)) {
				EntityID pEnt = this->map->objs.get(p.x, p.y);
				if (pEnt) {
					if (this->vault->registry.has<GameObject>(pEnt)) {
						Tile &pTile = this->vault->registry.get<Tile>(pEnt);
						GameObject &pObj = this->vault->registry.get<GameObject>(pEnt);
						if (pObj.team != obj.team) {
							// TODO: optimize
							Player &player = this->vault->registry.get<Player>(obj.player);
							player.enemyFound = true;
							player.enemyPos = p;

							if (distance(pTile.ppos, tile.ppos) < dist) {
								dist = distance(pTile.ppos, tile.ppos);
								finalTargetEnt = pEnt;
							}

						}
					}
				}
			}

			if (finalTargetEnt) {
				this->attack(unit, finalTargetEnt);
				unit.destpos = tile.pos;
			}
		} else {
			// change target if somebody nearer attack
			if (this->vault->registry.valid(unit.targetEnt)) {
				Tile &cpTile = this->vault->registry.get<Tile>(unit.targetEnt);
				EntityID finalTargetEnt = 0;
				float dist = std::numeric_limits<float>::max();

				for (sf::Vector2i const &p : this->tileSurfaceExtended(tile, obj.view)) {
					EntityID pEnt = this->map->objs.get(p.x, p.y);
					if (pEnt) {
						if (this->vault->registry.has<Unit>(pEnt)) {
							Tile &pTile = this->vault->registry.get<Tile>(pEnt);
							GameObject &pObj = this->vault->registry.get<GameObject>(pEnt);
							Unit &pUnit = this->vault->registry.get<Unit>(pEnt);
							if (pObj.team != obj.team && pUnit.targetEnt == entity && distance(pTile.ppos, tile.ppos) < distance(cpTile.ppos, tile.ppos)) {
								if (distance(pTile.ppos, tile.ppos) < dist) {
									dist = distance(pTile.ppos, tile.ppos);
									finalTargetEnt = pEnt;
								}
							}
						}
					}
				}

				if (finalTargetEnt) {
					this->attack(unit, finalTargetEnt);
					unit.destpos = tile.pos;
				}
			}
		}
	}

	// pass 2, calculate combat
	for (EntityID entity : view) {
		Tile &tile = view.get<Tile>(entity);
		Unit &unit = view.get<Unit>(entity);
		GameObject &obj = view.get<GameObject>(entity);
		if (obj.life > 0 && unit.targetEnt && this->vault->registry.valid(unit.targetEnt)) {
			int dist = 1;
			int maxDist = 1;
			if (unit.attack2.distance)
				dist = unit.attack2.distance;
			if (unit.attack2.maxDistance)
				maxDist = unit.attack2.maxDistance;

			Tile &destTile = this->vault->registry.get<Tile>(unit.targetEnt);
			GameObject &destObj = this->vault->registry.get<GameObject>(unit.targetEnt);

//					bool inRange = this->ennemyInRange(tile, destTile, dist, maxDist) || this->ennemyInRange(tile, destTile, 1, 1);
			bool inRange = this->ennemyInRange(tile, destTile, dist, maxDist) || this->ennemyInRange(tile, destTile, 1, 1);
			if (inRange) {
				int attackPower = unit.attack1.power;

#ifdef COMBAT_DEBUG
				std::cout << "CombatSystem: " << entity << " arrived at target, fight " << distance(tile.ppos, destTile.ppos) << " " << unit.targetEnt << std::endl;
#endif
				sf::Vector2i distDiff = (destTile.pos - tile.pos);
				// use attack2 if in correct range
				if (unit.attack2.distance && this->ennemyInRange(tile, destTile, dist, maxDist)) {
					attackPower = unit.attack2.power;
				}
				unit.destpos = tile.pos;
				unit.targetPos = tile.pos;

				float damage = (float)attackPower / 100.0f;

				// damage malus for moving target
				if (this->vault->registry.has<Unit>(unit.targetEnt)) {
					Unit &destUnit = this->vault->registry.get<Unit>(unit.targetEnt);
					damage /= length(destUnit.velocity) * 2.0f;
				}
#ifdef COMBAT_DEBUG
				std::cout << "CombatSystem: " << entity << " " << obj.name << " inflige " << damage << " to " << unit.targetEnt << std::endl;
#endif
				destObj.life -= damage;

				if (destObj.player) {
					this->addPlayerFrontPoint(destObj.player, unit.targetEnt, destTile.pos);
				}

				// start/continue attacking
				this->changeState(entity, "attack");
				unit.destpos = tile.pos;

			} else {
				if (unit.destpos == tile.pos) {
					sf::Vector2i dpos = destTile.pos;

					unit.targetPos = dpos;
					this->goTo(unit, dpos);

#ifdef COMBAT_DEBUG
					std::cout << "CombatSystem: " << entity << " new dest pos " << unit.destpos.x << "x" << unit.destpos.y << std::endl;
#endif
				}
			}
		} else {
			unit.targetEnt = 0;
		}
	}

	// pass 3 fx and die
	for (EntityID entity : view) {
		Tile &tile = view.get<Tile>(entity);
		Unit &unit = view.get<Unit>(entity);
		GameObject &obj = view.get<GameObject>(entity);

		if (obj.life <= 0) {
			if (tile.state == "die") {
				if ((rand() % 16) == 0 && this->vault->registry.has<Effects>(entity) && this->vault->registry.get<Effects>(entity).effects.count("alt_die")) {
					// alt die FX
					ParticleEffectOptions altOptions;
					altOptions.destPos = tile.ppos;
					altOptions.direction = 0;
					// copy shader options from original tile
					if (tile.shader) {
						// setting screenSize will create a RenderTexture, only do this for effect needing shader
						altOptions.screenSize = sf::Vector2i(this->screenWidth, this->screenHeight);
						altOptions.applyShader = true;
						altOptions.shader = this->vault->factory.shrManager.getRef(tile.shaderName);
						altOptions.shaderOptions = tile.shaderOptions;
					}

					this->emitEffect("alt_die", entity, tile.ppos, altOptions);
					obj.destroy = true;
				} else {
					// unit died, destroy after playing anim
					if (this->vault->registry.has<AnimatedSpritesheet>(entity))
					{
						AnimatedSpritesheet &anim = this->vault->registry.get<AnimatedSpritesheet>(entity);
						if (anim.states.count("die") > 0) {
							if (anim.states["die"][tile.view].l >= 1) {
								obj.destroy = true;
							}
						} else {
							obj.destroy = true;
						}
					}
				}
			} else {
				this->changeState(entity, "die");
				unit.targetEnt = 0;
				unit.destpos = tile.pos;
			}
		}

		if (tile.state == "attack") {
			// attacked obj does not exists anymore, stop attacking
			if (!unit.targetEnt || !this->vault->registry.valid(unit.targetEnt)) {
#ifdef COMBAT_DEBUG
				if (unit.targetEnt) {
					std::cout << "CombatSystem: " << entity << "enemy target does not exists anymore " << unit.targetEnt << std::endl;
				}
#endif
				if(unit.targetEnt) {
					Player &player = this->vault->registry.get<Player>(obj.player);
					player.kills.insert(unit.targetEnt);
				}

				this->changeState(entity, "idle");
				unit.targetEnt = 0;
				unit.destpos = tile.pos;
			}

		}
	}

	auto buildingView = this->vault->registry.persistent<Tile, GameObject, Building>();
	for (EntityID entity : buildingView) {
		Tile &tile = buildingView.get<Tile>(entity);
		Building &building = buildingView.get<Building>(entity);
		GameObject &obj = buildingView.get<GameObject>(entity);

		if (obj.life <= 0) {

			ParticleEffectOptions projOptions;
			projOptions.destPos = tile.ppos;
			projOptions.direction = 0;

			this->emitEffect("destroy", entity, tile.ppos, projOptions);
			map->sounds.push(SoundPlay{"explosion", 2, true, tile.pos});

			obj.destroy = true;

		} else {
			// change tile view to show damages
			if (obj.life < obj.maxLife * 0.75)
				tile.view = 1;
			if (obj.life < obj.maxLife * 0.50)
				tile.view = 2;
			if (obj.life < obj.maxLife * 0.25)
				tile.view = 3;
		}
	}
}