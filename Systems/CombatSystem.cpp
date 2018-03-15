#include "CombatSystem.hpp"

void CombatSystem::init() {
	this->vault->dispatcher.connect<TimerStarted>(this);
	this->vault->dispatcher.connect<TimerLooped>(this);
	this->vault->dispatcher.connect<TimerEnded>(this);
}

void CombatSystem::attacking(EntityID entity) {
	if (this->vault->registry.has<Unit>(entity)) {
		Tile &tile = vault->registry.get<Tile>(entity);
		if (tile.state == "attack") {
			Unit &unit = vault->registry.get<Unit>(entity);

#ifdef COMBAT_DEBUG
			std::cout << "CombatSystem: play sound " << unit.attackSound << std::endl;
#endif
			this->vault->dispatcher.trigger<SoundPlay>(unit.attackSound, 1, false, tile.pos);

			if (unit.targetEnt) {
				Tile &destTile = this->vault->registry.get<Tile>(unit.targetEnt);
				sf::Vector2f fxPos = destTile.ppos;
				sf::Vector2f diffPos = normalize(sf::Vector2f(tile.pos - destTile.pos)) * 16.0f;
				fxPos.x += diffPos.x;
				fxPos.y += diffPos.y;

				ParticleEffectOptions hitOptions;
				hitOptions.destPos = destTile.ppos;
				hitOptions.direction = getDirection(tile.pos, destTile.pos);

				this->vault->dispatcher.trigger<EffectCreate>("hit", unit.targetEnt, fxPos, hitOptions);

				if (this->vault->registry.has<Effects>(entity)) {
					Effects &effects = this->vault->registry.get<Effects>(entity);
					if (effects.effects.count("projectile") > 0) {
						ParticleEffectOptions projOptions;
						projOptions.destPos = destTile.ppos;
						projOptions.direction = getDirection(tile.pos, destTile.pos);

						this->vault->dispatcher.trigger<EffectCreate>("projectile", entity, tile.ppos, projOptions);

						float expectedDuration = length(destTile.ppos - tile.ppos) / 80.0f;
						this->vault->factory.createTimer(this->vault->registry, entity, "projectile_arrival", expectedDuration, false);
					}
				}


			} else {
				this->changeState(entity, "idle");
				unit.targetEnt = 0;
				unit.destpos = tile.pos;
			}
		}
	}
}

// frame changed
void CombatSystem::receive(const TimerLooped &event) {
	if (event.name == "attack")
		this->attacking(event.entity);
}

void CombatSystem::receive(const TimerStarted &event) {
	if (event.name == "attack")
		this->attacking(event.entity);
}

void CombatSystem::receive(const TimerEnded &event) {
	if (event.name == "projectile_arrival") {
		if (this->vault->registry.has<Timer>(event.entity)) {
			Timer &timer = this->vault->registry.get<Timer>(event.entity);
			if (this->vault->registry.has<Unit>(timer.emitterEntity)) {
				Unit &unit = this->vault->registry.get<Unit>(timer.emitterEntity);
				GameObject &obj = this->vault->registry.get<GameObject>(timer.emitterEntity);
				if (unit.targetEnt) {
					Tile &destTile = this->vault->registry.get<Tile>(unit.targetEnt);
					sf::Vector2i projDestPos = destTile.pos;
					switch (unit.special) {
					case SpecialSkillStr("destroy_nature"): {
						EntityID resEnt = this->map->resources.get(projDestPos.x, projDestPos.y);
						if (resEnt) {
							Resource &resource = this->vault->registry.get<Resource>(resEnt);
							if (resource.type == "nature") {
								this->map->resources.set(projDestPos.x, projDestPos.y, 0);
								this->vault->registry.destroy(resEnt);
								std::cout << "SpecialSkill: destroy nature at " << projDestPos << std::endl;
							}
						}
					}
					break;
					case SpecialSkillStr("seed_nature"): {
						if (!this->map->resources.get(projDestPos.x, projDestPos.y) && this->map->staticBuildable.get(projDestPos.x, projDestPos.y) == 0) {
							if ((rand() % 4) == 0) {
								EntityID resEnt = this->vault->factory.plantResource(this->vault->registry, "nature", projDestPos.x, projDestPos.y);
								this->map->resources.set(projDestPos.x, projDestPos.y, resEnt);
								std::cout << "SpecialSkill: plant nature at " << projDestPos << std::endl;
							}
						}
					}
					break;
					case SpecialSkillStr("collateral_projectile"): {
						for (int w = projDestPos.x - 1; w < projDestPos.x + 1; w++) {
							for (int h = projDestPos.y - 1; h < projDestPos.y + 1; h++) {
								EntityID colEnt = this->map->objs.get(w, h);
								if (colEnt) {
									GameObject &colObj = this->vault->registry.get<GameObject>(colEnt);
									if (colObj.team != obj.team) {
										colObj.life -= (float)unit.attack2.power / 16.0f;
										std::cout << "SpecialSkill: collateral projectile damage at " << w << "x" << h << " on " << colEnt << std::endl;
									}
								}
							}
						}
					}
					break;
					default:
						break;
					}
//					std::cout << "TIMER PROJECTILE AT " << destTile.ppos << std::endl;
				}
			}
		}
	} else if (event.name == "delayed_destroy") {
		if (this->vault->registry.has<Timer>(event.entity)) {
			Timer &timer = this->vault->registry.get<Timer>(event.entity);
			this->vault->dispatcher.trigger<EntityDelete>(timer.emitterEntity);
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

		if (unit.targetEnt) {
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
							if (this->vault->registry.has<Building>(destUnit.targetEnt)) {
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

	// pass 2, calculate combat
	for (EntityID entity : view) {
		Tile &tile = view.get<Tile>(entity);
		Unit &unit = view.get<Unit>(entity);
		GameObject &obj = view.get<GameObject>(entity);
		if (obj.life > 0 && unit.targetEnt) {
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

				float damage = (float)attackPower / 100.0f;

				// damage malus for moving target
				if (this->vault->registry.has<Unit>(unit.targetEnt)) {
					Unit &destUnit = this->vault->registry.get<Unit>(unit.targetEnt);
					damage /= length(destUnit.velocity) + 1.0f;
				}
#ifdef COMBAT_DEBUG
				std::cout << "CombatSystem: " << entity << " " << obj.name << " inflige " << damage << " to " << unit.targetEnt << std::endl;
#endif
				destObj.life -= damage;

				if (unit.special == SpecialSkillStr("collateral")) {
					for (int w = tile.pos.x - 1; w < tile.pos.x + 1; w++) {
						for (int h = tile.pos.y - 1; h < tile.pos.y + 1; h++) {
							EntityID colEnt = this->map->objs.get(w, h);
							if (colEnt) {
								GameObject &colObj = this->vault->registry.get<GameObject>(colEnt);
								if (colObj.team != obj.team) {
									colObj.life -= damage / 4.0f;
									std::cout << "SpecialSkill: collateral damage at " << w << "x" << h << " on " << colEnt << std::endl;
								}
							}
						}
					}
				}

				if (destObj.player) {
					this->addPlayerFrontPoint(destObj.player, unit.targetEnt, destTile.pos);
				}

				// start/continue attacking
				this->changeState(entity, "attack");
				unit.velocity = sf::Vector2f(0, 0);
				unit.destpos = tile.pos;

			} else {
//				if (unit.destpos == tile.pos) {
				sf::Vector2i dpos = destTile.pos;

				if (tile.state == "attack") // change to idle if attacking and out of range
					this->changeState(entity, "idle");

				this->goTo(unit, dpos);

#ifdef COMBAT_DEBUG
				std::cout << "CombatSystem: " << entity << " new dest pos " << unit.destpos.x << "x" << unit.destpos.y << std::endl;
#endif
//				}
			}
		}
	}

	// pass 3 fx and die
	for (EntityID entity : view) {
		Tile &tile = view.get<Tile>(entity);
		Unit &unit = view.get<Unit>(entity);
		GameObject &obj = view.get<GameObject>(entity);

		if (obj.life <= 0) {
			if (tile.state != "die") {
				if ((rand() % 16) == 0 && this->vault->registry.has<Effects>(entity) && this->vault->registry.get<Effects>(entity).effects.count("alt_die")) {
					// alt die FX
					ParticleEffectOptions altOptions;
					altOptions.destPos = tile.ppos;
					altOptions.direction = 0;
					// copy shader options from original tile
					if (tile.shader) {
						altOptions.applyShader = true;
						altOptions.shader = this->vault->factory.shrManager.getRef(tile.shaderName);
						altOptions.shaderOptions = tile.shaderOptions;
					}

					this->vault->dispatcher.trigger<EffectCreate>("alt_die", entity, tile.ppos, altOptions);

					this->vault->factory.createTimer(this->vault->registry, entity, "delayed_destroy", 5.0, false);

				} else {
					// unit died, destroy after playing anim
					if (this->vault->registry.has<AnimatedSpritesheet>(entity))
					{
						AnimatedSpritesheet &anim = this->vault->registry.get<AnimatedSpritesheet>(entity);
						if (anim.states.count("die") > 0) {
							AnimatedSpriteView &view = anim.states["die"][0];

							this->vault->factory.createTimer(this->vault->registry, entity, "delayed_destroy", view.duration * view.frames.size(), false);
						} else {
							this->vault->dispatcher.trigger<EntityDelete>(entity);
						}
					}
				}

				this->changeState(entity, "die");
				unit.targetEnt = 0;
				unit.destpos = tile.pos;
			}
		}

		if (tile.state == "attack") {
			// attacked obj does not exists anymore, stop attacking
			if (!unit.targetEnt) {
#ifdef COMBAT_DEBUG
				if (unit.targetEnt) {
					std::cout << "CombatSystem: " << entity << "enemy target does not exists anymore " << unit.targetEnt << std::endl;
				}
#endif
				if (unit.targetEnt) {
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
			if (tile.state != "destroy") {

				ParticleEffectOptions projOptions;
				projOptions.destPos = tile.ppos;
				projOptions.direction = 0;

				this->vault->dispatcher.trigger<EffectCreate>("destroy", entity, tile.ppos, projOptions);
				this->vault->dispatcher.trigger<SoundPlay>("explosion", 2, true, tile.pos);

				this->vault->factory.createTimer(this->vault->registry, entity, "delayed_destroy", 1.0f, false);

				tile.state = "destroy";
			}

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