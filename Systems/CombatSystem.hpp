#pragma once

#include "GameSystem.hpp"

#include "dbscan/dbscan.h"

class CombatSystem : public GameSystem {
public:

	void updateFront(float dt) {
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

	void update(float dt) {
		// pass 1, if an ennemy is in sight, then attack
		auto view = this->vault->registry.persistent<Tile, GameObject, Unit>();
		for (EntityID entity : view) {
			Tile &tile = view.get<Tile>(entity);
			GameObject &obj = view.get<GameObject>(entity);
			Unit &unit = view.get<Unit>(entity);
			if (!unit.destAttack) {
				std::vector<EntityID>targets;
				for (sf::Vector2i const &p : this->tileSurfaceExtended(tile, obj.view)) {
					EntityID pEnt = this->map->objs.get(p.x, p.y);
					if (pEnt) {
						if (this->vault->registry.has<GameObject>(pEnt)) {
							GameObject &pObj = this->vault->registry.get<GameObject>(pEnt);
							if (pObj.team != obj.team) {
								// TODO: optimize
								Player &player = this->vault->registry.get<Player>(obj.player);
								player.enemyFound = true;
								player.enemyPos = p;

								targets.push_back(pEnt);
							}
						}
					}
				}
				if (targets.size() > 0) {
					// attack random in sight target
					std::random_shuffle ( targets.begin(), targets.end() );
					this->attack(unit, targets.front());
				}
			}
		}

		// pass 2, calculate combat
		for (EntityID entity : view) {
			Tile &tile = view.get<Tile>(entity);
			Unit &unit = view.get<Unit>(entity);
			GameObject &obj = view.get<GameObject>(entity);
			if (unit.destAttack && obj.life > 0 && this->vault->registry.valid(unit.destAttack)) {
				int dist = 1;
				if (unit.attack2.distance)
					dist = unit.attack2.distance;

				Tile &destTile = this->vault->registry.get<Tile>(unit.destAttack);
				GameObject &destObj = this->vault->registry.get<GameObject>(unit.destAttack);

				bool inRange = false;
				for (sf::Vector2i const &p : this->tileAround(destTile, dist)) {
					if (tile.pos == p)
						inRange = true;
				}

				if (!inRange && tile.pos == unit.destAttackPos && tile.pos == unit.nextpos) {
#ifdef COMBAT_DEBUG
					std::cout << "CombatSystem: " << entity << " arrived but enemy is not in range anymore, wait a bit" << std::endl;
#endif
					sf::Vector2i dpos = this->nearestTileAround(tile.pos, destTile, dist);
					unit.destAttackPos = dpos;
					this->goTo(unit, dpos);
				} else {

					if (inRange) {
						if (tile.pos == unit.nextpos) { // unit must be arrived at a position
							int attackPower = unit.attack1.power;

#ifdef COMBAT_DEBUG
							std::cout << "CombatSystem: " << entity << " arrived at target, fight " << unit.destAttack << std::endl;
#endif
							sf::Vector2i distDiff = (destTile.pos - tile.pos);
							// use attack2 if in correct range
							if (dist > 1 && (abs(distDiff.x) == dist || abs(distDiff.y) == dist)) {
#ifdef COMBAT_DEBUG
								std::cout << "CombatSystem: " << entity << " " << obj.name << " use attack2 on " << unit.destAttack << " " << destObj.name << std::endl;
#endif
								attackPower = unit.attack2.power;
							}
							unit.destpos = tile.pos;
							unit.destAttackPos = tile.pos;

							float damage = (float)attackPower / 100.0;
#ifdef COMBAT_DEBUG
							std::cout << "CombatSystem: " << entity << " " << obj.name << " inflige " << damage << " to " << unit.destAttack << std::endl;
#endif
							destObj.life -= damage;

//							if(tile.state!="attack") {
							if (destObj.player) {
								Player &destPlayer = this->vault->registry.get<Player>(destObj.player);
								if (this->vault->registry.has<Unit>(unit.destAttack)) {
									if (this->approxDistance(destPlayer.initialPos, destTile.pos) < this->approxDistance(sf::Vector2i(0, 0), sf::Vector2i(this->map->width, this->map->height)) / 4) {
										destPlayer.allFrontPoints.push_back(destTile.pos);
									}
								} else {
									if (this->vault->registry.has<Building>(unit.destAttack)) {
										destPlayer.allFrontPoints.push_back(destTile.pos);
									}
								}
							}
//							}

							if (destObj.life <= 0) {
								destObj.life = 0;
								if (destTile.state != "die") {
									Player &player = this->vault->registry.get<Player>(obj.player);
									player.kills.insert(unit.destAttack);
								}
								if (this->vault->registry.has<Unit>(unit.destAttack)) {
									this->changeState(unit.destAttack, "die");
								}


							} else {
								tile.view = this->getDirection(tile.pos, destTile.pos);

								this->changeState(entity, "attack");
							}

							if (destObj.life == 0) {
								this->changeState(entity, "idle");
								unit.destAttack = 0;
								unit.destpos = tile.pos;
							} else {
								if (this->vault->registry.has<Unit>(unit.destAttack)) {
									Unit &destUnit = this->vault->registry.get<Unit>(unit.destAttack);
									if (destTile.state == "idle" || destTile.state == "move") {
										// if ennemy is idle, he will fight back
										this->attack(destUnit, entity);

									} else if (destTile.state == "attack" && destUnit.destAttack) {
										// if ennemy is attacking a building, he will fight back
										if (this->vault->registry.valid(destUnit.destAttack) && this->vault->registry.has<Building>(destUnit.destAttack)) {
											this->attack(destUnit, entity);
										}
									}
								}

							}
						}
					} else {
						sf::Vector2i dpos = this->nearestTileAround(tile.pos, destTile, dist);
#ifdef COMBAT_DEBUG
						std::cout << "CombatSystem: " << entity << " target out of range, go to " << dpos.x << "x" << dpos.y << std::endl;
#endif
						unit.destAttackPos = dpos;
						this->goTo(unit, dpos);
					}
				}
			} else {
				unit.destAttack = 0;
			}
		}

		// pass 3
		for (EntityID entity : view) {
			Tile &tile = view.get<Tile>(entity);
			Unit &unit = view.get<Unit>(entity);
			GameObject &obj = view.get<GameObject>(entity);

			if (obj.life == 0) {

				if (tile.state == "die" && (rand() % 8) == 0 && this->vault->registry.has<Effects>(entity) && this->vault->registry.get<Effects>(entity).effects.count("alt_die")) {
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

					this->emitEffect("alt_die", entity, tile.ppos, 2.0, altOptions);
					obj.destroy = true;
				} else {
					// unit died, destroy after playing anim
					if (this->vault->registry.has<AnimatedSpritesheet>(entity))
					{
						AnimatedSpritesheet &anim = this->vault->registry.get<AnimatedSpritesheet>(entity);
						if (anim.states[tile.state][tile.view].l >= 1) {
							obj.destroy = true;
						}
					}
				}

				this->changeState(entity, "die");
				unit.destAttack = 0;
				unit.destpos = tile.pos;
			}

			if (tile.state == "attack") {
				// play sound at frame 1
				if (this->vault->registry.has<AnimatedSpritesheet>(entity))
				{
					AnimatedSpritesheet &anim = this->vault->registry.get<AnimatedSpritesheet>(entity);

					anim.states[tile.state][tile.view].frameChangeCallback = [this, entity](int frame) {
						if (vault->registry.valid(entity)) {
							Unit &unit = vault->registry.get<Unit>(entity);
							Tile &tile = vault->registry.get<Tile>(entity);
							if (frame == 1) {
#ifdef COMBAT_DEBUG
								std::cout << "CombatSystem: play sound " << unit.attackSound << std::endl;
#endif
								map->sounds.push(SoundPlay {unit.attackSound, 1, false, tile.pos});

								if (unit.destAttack) {
									if (this->vault->registry.valid(unit.destAttack)) { // ???
										Tile &destTile = this->vault->registry.get<Tile>(unit.destAttack);
										sf::Vector2f fxPos = destTile.ppos;
										sf::Vector2i diffPos = tile.pos - destTile.pos;
										fxPos.x += diffPos.x * 8.0 + 16.0;
										fxPos.y += diffPos.y * 8.0;

										ParticleEffectOptions hitOptions;
										hitOptions.destPos = destTile.ppos;
										hitOptions.direction = this->getDirection(tile.pos, destTile.pos);
										hitOptions.screenSize = sf::Vector2i(this->screenWidth, this->screenHeight);

										this->emitEffect("hit", unit.destAttack, fxPos, 1.0, hitOptions);

										ParticleEffectOptions projOptions;
										projOptions.destPos = destTile.ppos;
										projOptions.direction = this->getDirection(tile.pos, destTile.pos);
										projOptions.screenSize = sf::Vector2i(this->screenWidth, this->screenHeight);

										this->emitEffect("projectile", entity, tile.ppos, 3.0, projOptions);
									} else {
										this->changeState(entity, "idle");
										unit.destAttack = 0;
										unit.destpos = tile.pos;
									}
								}
							}
						}
					};
				}

				// attacked obj does not exists anymore, stop attacking
				if (!unit.destAttack || !this->vault->registry.valid(unit.destAttack)) {
					this->changeState(entity, "idle");
					unit.destAttack = 0;
					unit.destpos = tile.pos;
				}

			}
		}

		auto buildingView = this->vault->registry.persistent<Tile, GameObject, Building>();
		for (EntityID entity : buildingView) {
			Tile &tile = buildingView.get<Tile>(entity);
			Building &building = buildingView.get<Building>(entity);
			GameObject &obj = buildingView.get<GameObject>(entity);

			if (obj.life == 0) {

				ParticleEffectOptions projOptions;
				projOptions.destPos = tile.ppos;
				projOptions.direction = 0;

				this->emitEffect("destroy", entity, tile.ppos, 1.0, projOptions);
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
};