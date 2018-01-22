#pragma once

#include "GameSystem.hpp"

class CombatSystem : public GameSystem {
public:
	void update(float dt) {

		// if an ennemy is in sight, then attack
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

//				float curDist = this->approxDistance(tile.pos, destTile.pos);
//				std::cout << "CombatSystem: "<<entity<< " "<<curDist<<"/"<<dist << " from target "<<unit.destAttack<<std::endl;

				bool inRange = false;
				for (sf::Vector2i p : this->tileAround(destTile, dist)) {
					if (tile.pos == p)
						inRange = true;
				}

				if (!inRange && tile.pos == unit.destAttackPos && tile.pos == unit.nextpos) {
//					std::cout << "CombatSystem: " << entity << " arrived but enemy is not in range anymore, wait a bit" << std::endl;
//					tile.state = "idle";
//					unit.destAttack = 0;
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
							if(dist > 1 && (abs(distDiff.x)==dist || abs(distDiff.y)==dist)) {
//								std::cout << "CombatSystem: " << entity << " " <<obj.name << " use attack2 on "<<unit.destAttack << " "<<destObj.name << std::endl; 
								attackPower = unit.attack2.power;
							}
							unit.destpos = tile.pos;
							unit.destAttackPos = tile.pos;
							destObj.life -= (attackPower * dt);
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
//					std::cout << "GO ATTACK " << entity << " " << unit.destpos.x << "x" << unit.destpos.y << std::endl;
					}
				}
			} else {
				unit.destAttack = 0;
			}
		}

		for (EntityID entity : view) {
			Tile &tile = view.get<Tile>(entity);
			Unit &unit = view.get<Unit>(entity);
			GameObject &obj = view.get<GameObject>(entity);

			if (obj.life == 0) {
				this->changeState(tile, "die");
				unit.destAttack = 0;
				unit.destpos = tile.pos;
			}

			if (tile.state == "attack") {
				if(tile.animHandlers["attack"].getCurrentFrame() == 0) {
//					std::cout << "play sound at "<< (int)(tile.animHandlers["attack"].t * 1000)<< std::endl;
					if(unit.attackSound.getStatus() != sf::Sound::Status::Playing)
					unit.attackSound.play();
				}
				if (!unit.destAttack || !this->vault->registry.valid(unit.destAttack)) {
					this->changeState(tile, "idle");
					unit.destAttack = 0;
					unit.destpos = tile.pos;
				}
			}
		}

	}
};