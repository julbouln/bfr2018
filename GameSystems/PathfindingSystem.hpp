#pragma once

#include "GameSystem.hpp"

//#define PATHFINDING_DEBUG

#define PATHFINDING_MAX_NO_PATH 16

class PathfindingSystem : public GameSystem {
public:
	void updatePathfindingLayer(float dt) {
		this->map->pathfinding.clear();
		auto buildingView = this->vault->registry.persistent<Tile, Building>();

		for (EntityID entity : buildingView) {
			Tile &tile = buildingView.get<Tile>(entity);

			for (sf::Vector2i const &p : this->tileSurface(tile)) {
				this->map->pathfinding.set(p.x, p.y, entity);
			}
		}

		auto unitView = this->vault->registry.persistent<Tile, Unit>();
		for (EntityID entity : unitView) {
			Tile &tile = unitView.get<Tile>(entity);
			Unit &unit = unitView.get<Unit>(entity);

			if (tile.pos == unit.destpos) {
				this->map->pathfinding.set(tile.pos.x, tile.pos.y, entity);
			}
		}

		auto decorView = this->vault->registry.persistent<Tile, Decor>();
		for (EntityID entity : decorView) {
			Tile &tile = decorView.get<Tile>(entity);
			Decor &decor = decorView.get<Decor>(entity);

			if (decor.blocking) {
				for (sf::Vector2i const &p : this->tileSurface(tile)) {
					this->map->pathfinding.set(p.x, p.y, entity);
				}
			}
		}

	}

	bool checkAround(EntityID entity, sf::Vector2i npos) {
		Tile &tile = this->vault->registry.get<Tile>(entity);

		for (sf::Vector2i const &p : this->tileSurfaceExtended(tile, 2)) {
			EntityID other = this->map->objs.get(p.x, p.y);
			if (other && other != entity) {
				if (this->vault->registry.has<Unit>(other)) {
					Unit &otherUnit = this->vault->registry.get<Unit>(other);
					Tile &otherTile = this->vault->registry.get<Tile>(other);
					if (otherTile.pos != otherUnit.destpos && (npos == otherUnit.nextpos || npos == otherTile.pos)) {
#ifdef PATHFINDING_DEBUG
						std::cout << "Pathfinding: " << other << " go to " << npos.x << "x" << npos.y << std::endl;
#endif
						return false;
					}
				}
			}
		}
		return true;
	}

	void update(float dt) {
		this->updatePathfindingLayer(dt);

		auto view = this->vault->registry.persistent<Tile, GameObject, Unit>();
		for (EntityID entity : view) {
			Tile &tile = view.get<Tile>(entity);
			GameObject &obj = view.get<GameObject>(entity);
			Unit &unit = view.get<Unit>(entity);

			if (tile.pos != unit.destpos && obj.life > 0) {
				int diffx = abs(tile.ppos.x - unit.nextpos.x * 32);
				int diffy = abs(tile.ppos.y - unit.nextpos.y * 32);
				if (diffx >= 0 && diffx <= 2 && diffy >= 0 && diffy <= 2) {
					tile.pos = unit.nextpos;
					this->map->objs.set(tile.pos.x, tile.pos.y, entity); // mark pos immediatly
					tile.ppos = sf::Vector2f(tile.pos) * (float)32.0;

					if (tile.pos != unit.destpos) {

						JPS::PathVector path;
						bool found = JPS::findPath(path, *this->map, tile.pos.x, tile.pos.y, unit.destpos.x, unit.destpos.y, 1);

						if (found) {
							sf::Vector2i cpos(tile.pos.x, tile.pos.y);
							sf::Vector2i npos(path.front().x, path.front().y);

#ifdef PATHFINDING_DEBUG
							std::cout << "Pathfinding: " << entity << " check around " << tile.pos.x << "x" << tile.pos.y << std::endl;
#endif
							if (this->checkAround(entity, npos)) {
								unit.nextpos = npos;

								tile.direction = this->getDirection(cpos, npos);
								this->changeState(tile, "move");


#ifdef PATHFINDING_DEBUG
								std::cout << "Pathfinding: " << entity << " at " << cpos.x << "x" << cpos.y << " next position " << npos.x << "x" << npos.y << "(" << npos.x - cpos.x << "x" << npos.y - cpos.y << ")" << std::endl;
#endif
							} else {
								this->changeState(tile, "idle");
#ifdef PATHFINDING_DEBUG
								std::cout << "Pathfinding: " << entity << " wait a moment " << std::endl;
#endif
							}
						} else {
#ifdef PATHFINDING_DEBUG
							std::cout << "Pathfinding: " << entity << " no path found" << std::endl;
#endif
							this->changeState(tile, "idle");
							unit.nopath++;
							if (unit.nopath > PATHFINDING_MAX_NO_PATH) {
								sf::Vector2i fp = this->firstFreePosition(unit.destpos);
#ifdef PATHFINDING_DEBUG
								std::cout << "Pathfinding: " << entity << " go to first free position " << fp.x << "x" << fp.y << std::endl;
#endif
								this->goTo(unit, fp);
							}
						}
					} else {
#ifdef PATHFINDING_DEBUG
						std::cout << "Pathfinding: " << entity << " at destination" << std::endl;
#endif
						this->changeState(tile, "idle");
					}
				} else {
					//if (tile.state == "move")
					{
						float speed = (float)unit.speed / 2.0;
						tile.ppos += this->dirMovement(tile.direction, speed);

					}

					if (abs(tile.ppos.x / 32.0 - tile.pos.x) > 1 || abs(tile.ppos.y / 32.0 - tile.pos.y) > 1) {
						// something wrong, realign
						GameObject &obj = this->vault->registry.get<GameObject>(entity);
						std::cout << "Pathfinding: SOMETHING WRONG WITH " << entity << " state:" << tile.state << " life:" << obj.life << " " << tile.pos.x << "x" << tile.pos.y << " -> " << unit.nextpos.x << "x" << unit.nextpos.y << std::endl;
						tile.ppos = sf::Vector2f(tile.pos) * (float)32.0;
//						unit.nextpos = tile.pos;
					}
				}
			}
		}

	}
};