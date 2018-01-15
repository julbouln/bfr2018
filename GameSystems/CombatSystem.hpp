#pragma once

#include "GameSystem.hpp"

class CombatSystem : public GameSystem {
public:
	void update(float dt) {
		auto view = this->vault->registry.persistent<Tile, Unit>();
		for (EntityID entity : view) {
			Tile &tile = view.get<Tile>(entity);
			Unit &unit = view.get<Unit>(entity);
			if (unit.destAttack && this->vault->registry.valid(unit.destAttack)) {
				int dist = 1;
				if (unit.attack2.distance)
					dist = unit.attack2.distance;
				Tile &destTile = this->vault->registry.get<Tile>(unit.destAttack);
				GameObject &destObj = this->vault->registry.get<GameObject>(unit.destAttack);

				sf::Vector2i dpos = this->nearestTileAround(tile.pos, destTile, dist);
				if (tile.pos == dpos) {
					std::cout << "REALLY ATTACK " << entity << " " << destObj.life << std::endl;
					tile.direction = this->getDirection(tile.pos, destTile.pos);
					tile.state = "attack";
					destObj.life -= (unit.attack1.power * dt);
					if (destObj.life < 0)
						destObj.life = 0;

					if (destObj.life == 0) {
						tile.state = "idle";
						unit.destAttack = 0;

						destTile.state = "die";
					}
				} else {
					unit.destpos = dpos;
					unit.nopath = 0;
					std::cout << "GO ATTACK " << entity << " " << unit.destpos.x << "x" << unit.destpos.y << std::endl;
				}
			} else {
				unit.destAttack = 0;
			}
		}

		auto objView = this->vault->registry.persistent<Tile, GameObject>();
		for (EntityID entity : objView) {
			Tile &tile = objView.get<Tile>(entity);
			GameObject &obj = objView.get<GameObject>(entity);
			if (obj.life == 0 && tile.animHandlers[tile.state].l >= 1) {
				this->vault->registry.destroy(entity);
			}
		}
	}

};