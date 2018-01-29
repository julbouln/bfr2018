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

//							}

							if (destObj.life <= 0) {
								destObj.life = 0;
								if (destTile.state != "die") {
									Player &player = this->vault->registry.get<Player>(obj.player);
									player.kills.insert(unit.destAttack);
								}
								if (this->vault->registry.has<Unit>(unit.destAttack)) {
									this->changeState(destTile, "die");
									AnimationHandler &dieAnim = destTile.animHandlers["die"];
									dieAnim.getAnim().repeat = false;
								}


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

				if (tile.state == "die" && (rand()%8)==0 && this->vault->registry.has<Effects>(entity) && this->vault->registry.get<Effects>(entity).effects.count("alt_die")) {
					// alt die FX
					ParticleEffectOptions projOptions;
					projOptions.destPos = tile.ppos;
					projOptions.direction = 0;

					this->emitEffect("alt_die", entity, tile.ppos, 2.0, projOptions);
					obj.destroy = true;
				} else {
					// unit died, destroy after playing anim
					if (tile.animHandlers[tile.state].l >= 1) {
						obj.destroy = true;
					}
				}

				this->changeState(tile, "die");
				unit.destAttack = 0;
				unit.destpos = tile.pos;
			}

			if (tile.state == "attack") {
				// play sound at frame 1
				tile.animHandlers["attack"].changeFrameCallback = [this, entity](int frame) {
					if (vault->registry.valid(entity)) {
						Unit &unit = vault->registry.get<Unit>(entity);
						Tile &tile = vault->registry.get<Tile>(entity);
						if (frame == 1) {
#ifdef COMBAT_DEBUG
							std::cout << "CombatSystem: play sound " << unit.attackSound << std::endl;
#endif
							if (map->sounds.size() < MAX_SOUNDS)
								map->sounds.push(SoundPlay{unit.attackSound, 1, false, tile.pos});

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
									hitOptions.screenSize = sf::Vector2i(this->screenWidth,this->screenHeight);

									this->emitEffect("hit", unit.destAttack, fxPos, 1.0, hitOptions);

									ParticleEffectOptions projOptions;
									projOptions.destPos = destTile.ppos;
									projOptions.direction = this->getDirection(tile.pos, destTile.pos);
									projOptions.screenSize = sf::Vector2i(this->screenWidth,this->screenHeight);

									this->emitEffect("projectile", entity, tile.ppos, 3.0, projOptions);
								} else {
									this->changeState(tile, "idle");
									unit.destAttack = 0;
									unit.destpos = tile.pos;
								}

							}

						}
					}
				};

				// attacked obj does not exists anymore, stop attacking
				if (!unit.destAttack || !this->vault->registry.valid(unit.destAttack)) {
					this->changeState(tile, "idle");
					unit.destAttack = 0;
					unit.destpos = tile.pos;
				}

			}
		}

		auto buldingView = this->vault->registry.persistent<Tile, GameObject, Building>();
		for (EntityID entity : buldingView) {
			Tile &tile = buldingView.get<Tile>(entity);
			Building &building = buldingView.get<Building>(entity);
			GameObject &obj = buldingView.get<GameObject>(entity);

			if (obj.life == 0) {

				ParticleEffectOptions projOptions;
				projOptions.destPos = tile.ppos;
				projOptions.direction = 0;

				this->emitEffect("destroy", entity, tile.ppos, 1.0, projOptions);
				map->sounds.push(SoundPlay{"explosion", 1, true, tile.pos});

				obj.destroy = true;

			}

		}


	}

};