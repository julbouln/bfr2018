#pragma once

#include "GameVault.hpp"
#include "System.hpp"
#include "Map.hpp"

//#define GAME_SYSTEM_DEBUG

class GameSystem : public System {
public:
	Map *map;

	sf::Vector2i tilePosition(Tile &tile, sf::Vector2i p) {
		return sf::Vector2i(tile.pos.x + (p.x - tile.size.x / 2) + tile.offset.x,
		                    tile.pos.y + (p.y - tile.size.y / 2) + tile.offset.y);
	}

	std::vector<sf::Vector2i> tileSurface(Tile &tile) {
		std::vector<sf::Vector2i> surface;
		for (int w = 0; w < tile.size.x; w++) {
			for (int h = 0; h < tile.size.y; h++) {
				sf::Vector2i p = this->tilePosition(tile, sf::Vector2i(w, h));
				if (this->map->bound(p.x, p.y))
					surface.push_back(p);
			}
		}
		return surface;
	}

	std::vector<sf::Vector2i> vectorSurfaceExtended(sf::Vector2i pos, int dist) {
		std::vector<sf::Vector2i> surface;
		for (int w = -dist; w < dist + 1; w++) {
			for (int h = -dist; h < dist + 1; h++) {
				sf::Vector2i p(pos.x + w, pos.y + h);
				if (this->map->bound(p.x, p.y))
					surface.push_back(p);
			}
		}
		return surface;
	}

	std::vector<sf::Vector2i> tileSurfaceExtended(Tile &tile, int dist) {
		std::vector<sf::Vector2i> surface;
		for (int w = -dist; w < tile.size.x + dist; w++) {
			for (int h = -dist; h < tile.size.y + dist; h++) {
				sf::Vector2i p = this->tilePosition(tile, sf::Vector2i(w, h));
				if (this->map->bound(p.x, p.y))
					surface.push_back(p);
			}
		}
		return surface;
	}

	std::vector<sf::Vector2i> tileAround(Tile &tile, int dist) {
		std::vector<sf::Vector2i> surface;
		for (int w = -dist; w < tile.size.x + dist; w++) {
			for (int h = -dist; h < tile.size.y + dist; h++) {
				if (w <= -dist || h <= -dist || w >= tile.size.x || h >= tile.size.y) {
					sf::Vector2i p = this->tilePosition(tile, sf::Vector2i(w, h));
					if (this->map->bound(p.x, p.y))
						surface.push_back(p);
				}
			}
		}
		return surface;
	}

	sf::Vector2i nearestTileAround(sf::Vector2i src, Tile &tile, int dist) {
		sf::Vector2i nearest(1024, 1024);
		for (sf::Vector2i p : this->tileAround(tile, dist)) {
//			float mod = 1.0;
//			if (this->map->objs.get(p.x, p.y)) {
//				mod = 2.0;
//			} else {
//				std::cout << "DEBUG: free pos " << p.x << "x" << p.y << " around " << tile.pos.x << "x" << tile.pos.y << " (" << tile.size.x << "x" << tile.size.y << ")" << std::endl;
//			}
			if (!this->map->objs.get(p.x, p.y)) {
				if (this->approxDistance(src, p) < this->approxDistance(src, nearest)) {
//					std::cout << "DEBUG: free pos " << p.x << "x" << p.y << " around " << tile.pos.x << "x" << tile.pos.y << " (" << tile.size.x << "x" << tile.size.y << ")" << std::endl;
					nearest = p;
				} else {
				}
			}
		}
		if (nearest.x == 1024 && nearest.y == 1024) {
			std::cout << "BUG: no nearest pos around " << tile.pos.x << "x" << tile.pos.y << std::endl;
		}
		return nearest;
	}

	sf::Vector2i firstFreePosition(sf::Vector2i src) {
		sf::Vector2i fp;
		int dist = 1;
		while (dist < 16) {
			for (int w = -dist; w < dist + 1; w++) {
				for (int h = -dist; h < dist + 1; h++) {
					if (w == -dist || h == -dist || w == dist || h == dist) {
						int x = w + src.x;
						int y = h + src.y;
						if (this->map->bound(x, y)) {
							if (!this->map->objs.get(x, y))
								return sf::Vector2i(x, y);
						}
					}
				}
			}
			dist++;
		}
	}

	EntityID ennemyAtPosition(EntityID playerEnt, int x, int y) {
		Player &player = this->vault->registry.get<Player>(playerEnt);
		EntityID destEnt = this->map->objs.get(x, y);
		if (destEnt) {
			GameObject &obj = this->vault->registry.get<GameObject>(destEnt);
			if (obj.team != player.team)
				return destEnt;
		}
		return 0;
	}

	std::vector<sf::Vector2i> canBuild(EntityID playerEnt, EntityID entity) {
		Tile &tile = this->vault->registry.get<Tile>(entity);
		Player &player = this->vault->registry.get<Player>(playerEnt);
		std::vector<sf::Vector2i> restrictedPos;

		for (sf::Vector2i p : this->tileSurface(tile)) {
			EntityID pEnt = this->map->objs.get(p.x, p.y);
			if ((pEnt && pEnt != entity) || player.fog.get(p.x, p.y) == FogState::Unvisited || this->map->staticPathfinding.get(p.x,p.y) == 0)
			{
//				std::cout << "RESTRICT BUILD "<<p.x<<"x"<<p.y<<std::endl;
				restrictedPos.push_back(p);
			}
		}

		return restrictedPos;
	}

	sf::Vector2f dirMovement(int direction, float speed) {
		sf::Vector2f mov(0.0, 0.0);
		switch (direction) {
		case North:
			mov.y -= speed;
			break;
		case NorthEast:
			mov.x += speed;
			mov.y -= speed;
			break;
		case East:
			mov.x += speed;
			break;
		case SouthEast:
			mov.x += speed;
			mov.y += speed;
			break;
		case South:
			mov.y += speed;
			break;
		case NorthWest:
			mov.x -= speed;
			mov.y -= speed;
			break;
		case West:
			mov.x -= speed;
			break;
		case SouthWest:
			mov.x -= speed;
			mov.y += speed;
			break;
		default:
			break;
		}
		return mov;
	}


	std::vector<sf::Vector2f> lineTrajectory( int x0, int y0, int x1, int y1)
	{
		std::vector<sf::Vector2f> points;
		float dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
		float dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
		float err = (dx > dy ? dx : -dy) / 2, e2;

		for (;;) {
			points.push_back(sf::Vector2f(x0, y0));
			if (x0 == x1 && y0 == y1) break;
			e2 = err;
			if (e2 > -dx) { err -= dy; x0 += sx; }
			if (e2 < dy) { err += dx; y0 += sy; }
		}
		return points;
	}

	bool canSpendResources(EntityID playerEnt, ResourceType type, int val) {
		Player &player = this->vault->registry.get<Player>(playerEnt);
		if (player.resources > val)
			return true;
		else
			return false;
	}

	void spendResources(EntityID playerEnt, ResourceType type, int val) {
		Player &player = this->vault->registry.get<Player>(playerEnt);
		int spended = val;
		auto view = this->vault->registry.view<Resource>();
#ifdef GAME_SYSTEM_DEBUG
		std::cout << "GameSystem: spend " << val << " " << (int)type << std::endl;
#endif
		for (EntityID entity : view) {
			Resource &resource = view.get(entity);
			if (resource.type == type && resource.level > 0) {
				spended -= resource.level;
				this->vault->factory.destroyEntity(this->vault->registry, entity);

				if (spended <= 0)
					return;
			}
		}
	}

	// get object initial life from XML
	float objTypeLife(std::string type) {
		GameObject obj;
		this->vault->factory.parseGameObjectFromXml(type, obj);
		return obj.life;
	}

	void changeState(Tile &tile, std::string state) {
		tile.state = state;
		/*		AnimationHandler &currentAnim = tile.animHandlers[tile.state];

				currentAnim.changeAnim(tile.direction);
				currentAnim.update(0);
				tile.sprite.setTextureRect(currentAnim.bounds);
				*/
	}

	void playSound(sf::Sound &snd, std::string name) {
		snd.setBuffer(this->vault->factory.getSndBuf(name));
		snd.play();
	}

	void playRandomUnitSound(EntityID ent, std::string state) {
		Unit &unit = this->vault->registry.get<Unit>(ent);
		GameObject &obj = this->vault->registry.get<GameObject>(ent);

		this->playRandomUnitSound(obj, unit, state);
	}

	void playRandomUnitSound(GameObject &obj, Unit &unit, std::string state) {
		if (unit.soundActions[state] > 0) {
			int rnd = rand() % unit.soundActions[state];
			unit.sound.setBuffer(this->vault->factory.getSndBuf(obj.name + "_" + state + "_" + std::to_string(rnd)));
			unit.sound.play();
		}
	}

// action

	void seedResources(ResourceType type, EntityID entity) {
		if (this->vault->registry.valid(entity) && this->vault->registry.has<Tile>(entity)) { // FIXME: weird
			Tile &tile = this->vault->registry.get<Tile>(entity);
			for (sf::Vector2i p : this->tileAround(tile, 1)) {
				float rnd = ((float) rand()) / (float) RAND_MAX;
				if (rnd > 0.85) {
					if (!this->map->resources.get(p.x, p.y) && !this->map->objs.get(p.x, p.y)) {
//					std::cout << " seed "<<(int)type<< " at "<<p.x<<"x"<<p.y<<std::endl;
						EntityID resEnt = this->vault->factory.plantResource(this->vault->registry, type, p.x, p.y);
						this->map->resources.set(p.x, p.y, resEnt);
					}
				}
			}
		} else {
			std::cout << "BUG: seedResources entity " << entity << " invalid or does not has Tile" << std::endl;
		}
	}

	bool trainUnit(std::string type, EntityID playerEnt, EntityID entity ) {
		if (this->vault->registry.valid(entity) && this->vault->registry.has<Tile>(entity)) { // FIXME: weird
			Player &player = this->vault->registry.get<Player>(playerEnt);
			Tile &tile = this->vault->registry.get<Tile>(entity);

			if (this->canSpendResources(playerEnt, player.resourceType, 10)) {
				for (sf::Vector2i p : this->tileAround(tile, 1)) {
					if (!this->map->objs.get(p.x, p.y)) {
						EntityID newEnt = this->vault->factory.createUnit(this->vault->registry, playerEnt, type, p.x, p.y);
						this->spendResources(playerEnt, player.resourceType, 2 * this->objTypeLife(type));
#ifdef GAME_SYSTEM_DEBUG
						std::cout << "GameSystem: train " << type << std::endl;
#endif
						return true;
					}
				}
			}
		} else {
			std::cout << "BUG: trainUnit entity " << entity << " invalid or does not has Tile" << std::endl;
		}
		return false;
	}

	void goTo(Unit &unit, sf::Vector2i destpos) {
		unit.destpos = destpos;
		unit.nopath = 0;
	}

	void goTo(EntityID entity, sf::Vector2i destpos) {
		Unit &unit = this->vault->registry.get<Unit>(entity);
		this->goTo(unit, destpos);
	}

	void attack(Unit &unit, EntityID destEnt) {
		unit.destAttack = destEnt;
	}

	void attack(EntityID entity, EntityID destEnt) {
		Unit &unit = this->vault->registry.get<Unit>(entity);
		this->attack(unit, destEnt);
	}

};