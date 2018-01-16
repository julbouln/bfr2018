#pragma once

#include "GameSystem.hpp"

class PathfindingSystem : public GameSystem {
public:
	void update(float dt) {
		auto view = this->vault->registry.persistent<Tile, Unit>();
		for (EntityID entity : view) {
			Tile &tile = view.get<Tile>(entity);
			Unit &unit = view.get<Unit>(entity);

			if (tile.pos != unit.destpos) {
				int diffx = abs(tile.ppos.x - unit.nextpos.x * 32);
				int diffy = abs(tile.ppos.y - unit.nextpos.y * 32);
				if (diffx >= 0 && diffx <= 2 && diffy >= 0 && diffy <= 2) {
					tile.pos = unit.nextpos;
					//map.objs.set(tile.pos.x, tile.pos.y, entity);
					tile.ppos = sf::Vector2f(tile.pos) * (float)32.0;

					if (tile.pos != unit.destpos) {

						JPS::PathVector path;
						bool found = JPS::findPath(path, *this->map, tile.pos.x, tile.pos.y, unit.destpos.x, unit.destpos.y, 1);

						if (found) {
							int cx = tile.pos.x;
							int cy = tile.pos.y;

							int nx = path.front().x;
							int ny = path.front().y;

							unit.nextpos.x = nx;
							unit.nextpos.y = ny;

							this->map->objs.set(unit.nextpos.x, unit.nextpos.y, entity); // mark next pos as blocking

							tile.direction = this->getDirection(sf::Vector2i(cx, cy), sf::Vector2i(nx, ny));
							tile.state = "move";

							std::cout << "Pathfinding: " << entity << " next pos " << nx << "x" << ny << "(" << nx - cx << "x" << ny - cy << ")" << std::endl;
						} else {
							std::cout << "Pathfinding: " << entity << " no path found" << std::endl;
							tile.state = "idle";
							unit.nopath++;
							if (unit.nopath > 16) {
								sf::Vector2i fp = this->firstFreePosition(unit.destpos);
								std::cout << "first free pos " << fp.x << "x" << fp.y << std::endl;
								this->goTo(unit, fp);
							}
						}
					} else {
						std::cout << "Pathfinding: " << entity << " at destination" << std::endl;
						tile.state = "idle";
					}
				} else {
					float speed = (float)unit.speed / 2.0;
					switch (tile.direction) {
					case North:
						tile.ppos.y -= speed;
						break;
					case NorthEast:
						tile.ppos.x += speed;
						tile.ppos.y -= speed;
						break;
					case East:
						tile.ppos.x += speed;
						break;
					case SouthEast:
						tile.ppos.x += speed;
						tile.ppos.y += speed;
						break;
					case South:
						tile.ppos.y += speed;
						break;
					case NorthWest:
						tile.ppos.x -= speed;
						tile.ppos.y -= speed;
						break;
					case West:
						tile.ppos.x -= speed;
						break;
					case SouthWest:
						tile.ppos.x -= speed;
						tile.ppos.y += speed;
						break;
					}

					if (abs(tile.ppos.x / 32.0 - tile.pos.x) > 1 || abs(tile.ppos.y / 32.0 - tile.pos.y) > 1) {
						// something wrong, realign
						GameObject &obj = this->vault->registry.get<GameObject>(entity);
						std::cout << "Pathfinding: SOMETHING WRONG WITH "<<entity<< " state:"<<tile.state << " life:"<<obj.life<<std::endl;
						tile.ppos = sf::Vector2f(tile.pos) * (float)32.0;

					}

				}
			}
		}

	}
};