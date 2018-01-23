#pragma once

#include "GameSystem.hpp"

class CombatSystem : public GameSystem {
public:
	void update(float dt) {

		// pass 1, if an ennemy is in sight, then attack
		auto view = this->vault->registry.persistent<Tile, GameObject, Unit>();
		for (EntityID entity : view) {
			Tile &tile = view.get<Tile>(entity);
			GameObject &obj = view.get<GameObject>(entity);
			Unit &unit = view.get<Unit>(entity);
			if (!unit.destAttack) {
				std::vector<EntityID>targets;
				for (sf::Vector2i p : this->tileSurfaceExtended(tile, obj.view)) {
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

//		auto view = this->vault->registry.persistent<Tile, Unit>();
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
				for (sf::Vector2i p : this->tileAround(destTile, dist)) {
					if (tile.pos == p)
						inRange = true;
				}

				if (!inRange && tile.pos == unit.destAttackPos && tile.pos == unit.nextpos) {
//					std::cout << "CombatSystem: " << entity << " arrived but enemy is not in range anymore, wait a bit" << std::endl;
					sf::Vector2i dpos = this->nearestTileAround(tile.pos, destTile, dist);
					unit.destAttackPos = dpos;
					this->goTo(unit, dpos);
				} else {

					if (inRange) {
						if (tile.pos == unit.nextpos) { // unit must be arrived at a position
							int attackPower = unit.attack1.power;
//					std::cout << "CombatSystem: "<<entity <<" arrived at target, fight "<<unit.destAttack<<std::endl;
							sf::Vector2i distDiff = (destTile.pos - tile.pos);
							// use attack2 if in correct range
							if (dist > 1 && (abs(distDiff.x) == dist || abs(distDiff.y) == dist)) {
//								std::cout << "CombatSystem: " << entity << " " <<obj.name << " use attack2 on "<<unit.destAttack << " "<<destObj.name << std::endl;
								attackPower = unit.attack2.power;
							}
							unit.destpos = tile.pos;
							unit.destAttackPos = tile.pos;
							if (tile.state == "attack" && tile.animHandlers["attack"].getCurrentFrame() == 0) {
								int frCnt = tile.animHandlers["attack"].getAnim().getLength();
//								std::cout << obj.name << " ANIM LEN "<<frCnt<<std::endl;
								destObj.life -= (float)attackPower / 100.0 * frCnt;
							}
							if (destObj.life <= 0) {
								destObj.life = 0;
								if (destTile.state != "die") {
									Player &player = this->vault->registry.get<Player>(obj.player);
									player.kills.insert(unit.destAttack);
								}
								if (this->vault->registry.has<Unit>(unit.destAttack))
									destTile.state = "die";

							} else {
								tile.direction = this->getDirection(tile.pos, destTile.pos);
								this->changeState(tile, "attack");
							}

							if (destObj.life == 0) {
								this->changeState(tile, "idle");
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
										if (this->vault->registry.has<Building>(destUnit.destAttack)) {
											this->attack(destUnit, entity);
										}
									}
								}

							}
						}
					} else {
						sf::Vector2i dpos = this->nearestTileAround(tile.pos, destTile, dist);
//					std::cout << "CombatSystem: "<<entity <<" target out of range, go to "<<dpos.x<<"x"<<dpos.y<<std::endl;
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
				// unit died, destroy after playing anim
				if (tile.state == "die" && tile.animHandlers[tile.state].l >= 1) {
					obj.destroy = true;
				}

				this->changeState(tile, "die");
				unit.destAttack = 0;
				unit.destpos = tile.pos;
			}

			if (tile.state == "attack") {
				// play sound at frame 1
				if (tile.animHandlers["attack"].getCurrentFrame() == 1) {
//					std::cout << "play sound at "<< tile.animHandlers["attack"].getCurrentFrame()<< std::endl;
					if (unit.attackSound.getStatus() != sf::Sound::Status::Playing)
						unit.attackSound.play();
				}

				// attacked obj does not exists anymore, stop attacking
				if (!unit.destAttack || !this->vault->registry.valid(unit.destAttack)) {
					this->changeState(tile, "idle");
					unit.destAttack = 0;
					unit.destpos = tile.pos;
				}


				// projectile effect
				if (obj.effects.count("projectile") > 0) {
					EntityID projEnt = obj.effects["projectile"];
					MapEffect &proj = this->vault->registry.get<MapEffect>(projEnt);
					Tile &projTile = this->vault->registry.get<Tile>(projEnt);
					if (unit.destAttack) {
//						std::cout << "CombatSystem: show projectile "<<projEnt<<std::endl;
						Tile &destTile = this->vault->registry.get<Tile>(unit.destAttack);

						if (!proj.show) {
							proj.show = true;
							projTile.pos = tile.pos;
							projTile.ppos = tile.ppos;

							projTile.direction = this->getDirection(projTile.pos, destTile.pos);
							proj.speed = 3.0;

							proj.positions = this->lineTrajectory(projTile.ppos.x, projTile.ppos.y, destTile.ppos.x, destTile.ppos.y);
							proj.curPosition = 0;

							projTile.animHandlers["fx"].reset();
							projTile.animHandlers["fx"].set(0);

//							std::cout << "Trajectory: " << entity << " " << points.size() << std::endl;
//							for (sf::Vector2f p : points) {
//								std::cout << "Trajectory point: " << entity << " " << p.x << "x" << p.y << std::endl;
//							}
						} else {
							sf::Vector2i pos(tile.ppos.x / 32, tile.ppos.y / 32);
							projTile.pos = pos;

							if (projTile.pos == destTile.pos) {
								proj.speed = 0.0;
								proj.show = false;
								projTile.pos = tile.pos;
								projTile.ppos = tile.ppos;
								proj.curPosition = 0;

							}
						}
					}

				}


			} else {
				if (obj.effects.count("projectile") > 0) {
					EntityID projEnt = obj.effects["projectile"];
					MapEffect &proj = this->vault->registry.get<MapEffect>(projEnt);
					proj.show = false;
					proj.speed = 0.0;
				}
			}
		}

		auto buldingView = this->vault->registry.persistent<Tile, GameObject, Building>();
		for (EntityID entity : buldingView) {
			Tile &tile = buldingView.get<Tile>(entity);
			Building &building = buldingView.get<Building>(entity);
			GameObject &obj = buldingView.get<GameObject>(entity);

			if (obj.life == 0) {
				EntityID explosionEnt = obj.effects["explosion"];
				MapEffect &explosion = this->vault->registry.get<MapEffect>(explosionEnt);
				Tile &explosionTile = this->vault->registry.get<Tile>(explosionEnt);

				if (!explosion.show) {
					std::cout << "CombatSystem: " << entity << " destroyed, show explosion anim" << std::endl;
					explosion.show = true;
					explosionTile.pos = tile.pos;
					explosionTile.ppos = tile.ppos;
					explosionTile.animHandlers["fx"].reset();
					explosionTile.animHandlers["fx"].changeAnim(0);
					explosionTile.animHandlers["fx"].set(0);
					explosion.sound.play();
				}
//					std::cout << "CombatSystem: explosion "<<explosionTile.animHandlers["fx"].l<<std::endl;

				if (explosionTile.animHandlers["fx"].l >= 1) {
					obj.destroy = true;
				}

			}

		}


	}

	void updateProjectiles(float dt) {
		auto fxView = this->vault->registry.persistent<Tile, MapEffect>();
		for (EntityID entity : fxView) {
			Tile &projTile = fxView.get<Tile>(entity);
			MapEffect &proj = fxView.get<MapEffect>(entity);
			if (proj.show) {
				if (proj.positions.size() > 0) {
					if (proj.curPosition < proj.positions.size()) {
						projTile.ppos = proj.positions[proj.curPosition];
						proj.curPosition += proj.speed;
					} else {
						projTile.ppos = proj.positions[0];
						proj.curPosition = 0;
													projTile.animHandlers["fx"].reset();
							projTile.animHandlers["fx"].set(0);


					}
//					std::cout << "Projectile: " << entity << " " << proj.curPosition << "/"<<proj.positions.size()<< " "<< projTile.ppos.x << "x" << projTile.ppos.y << " " << projTile.pos.x << "x" << projTile.pos.y << std::endl;
				}
//				projTile.ppos += this->dirMovement(projTile.direction, effect.speed);
			}
		}
	}
};