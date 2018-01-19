#pragma once

#include "GameSystem.hpp"

//#define PATHFINDING_DEBUG

#define PATHFINDING_MAX_NO_PATH 16

class PathfindingSystem : public GameSystem {
public:
	void update(float dt) {
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
							this->changeState(tile, "move");


#ifdef PATHFINDING_DEBUG
							std::cout << "Pathfinding: " << entity << " next position " << nx << "x" << ny << "(" << nx - cx << "x" << ny - cy << ")" << std::endl;
#endif
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

					}

					if (abs(tile.ppos.x / 32.0 - tile.pos.x) > 1 || abs(tile.ppos.y / 32.0 - tile.pos.y) > 1) {
						// something wrong, realign
						GameObject &obj = this->vault->registry.get<GameObject>(entity);
						std::cout << "Pathfinding: SOMETHING WRONG WITH " << entity << " state:" << tile.state << " life:" << obj.life << " "<< tile.pos.x << "x"<<tile.pos.y<< " -> "<<unit.nextpos.x<<"x"<<unit.nextpos.y<< std::endl;
						tile.ppos = sf::Vector2f(tile.pos) * (float)32.0;
//						unit.nextpos = tile.pos;
					}
				}
			}
		}

	}
};