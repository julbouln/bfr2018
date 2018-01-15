#pragma once

#include "GameVault.hpp"
#include "System.hpp"
#include "Map.hpp"

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


	std::vector<sf::Vector2i> tileSurfaceExtended(Tile &tile) {
		std::vector<sf::Vector2i> surface;
		for (int w = -1; w < tile.size.x + 1; w++) {
			for (int h = -1; h < tile.size.y + 1; h++) {
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
				if (w <= -1 || h <= -1 || w >= tile.size.x || h >= tile.size.y) {
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

		for (int w = -dist; w < tile.size.x + dist; w++) {
			for (int h = -dist; h < tile.size.y + dist; h++) {
				if (w <= -1 || h <= -1 || w >= tile.size.x || h >= tile.size.y) {
					sf::Vector2i p = this->tilePosition(tile, sf::Vector2i(w, h));
					if (this->map->bound(p.x, p.y))
					{
						if (this->approxDistance(src, p) < this->approxDistance(src, nearest))
							nearest = p;
					}
				}
			}
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

	bool spendResources(EntityID playerEnt, ResourceType type, int val) {
		Player &player = this->vault->registry.get<Player>(playerEnt);
		if (player.resources > val) {
			auto view = this->vault->registry.view<Resource>();
			for (EntityID entity : view) {
				Resource &resource = view.get(entity);
				if (resource.type == type) {
					val -= resource.level;
					this->vault->registry.destroy(entity);
					if (val <= 0)
						break;
				}
			}
			return true;
		} else {
			return false;
		}
	}

	void seedResources(ResourceType type, EntityID entity) {
		Tile &tile = this->vault->registry.get<Tile>(entity);
		for (sf::Vector2i p : this->tileAround(tile, 1)) {
			float rnd = ((float) rand()) / (float) RAND_MAX;
			if (rnd > 0.8) {
				if (!this->map->resources.get(p.x, p.y) && !this->map->objs.get(p.x, p.y))
					this->vault->factory.plantResource(this->vault->registry, type, p.x, p.y);
			}
		}
	}

	bool trainUnit(std::string type, EntityID playerEnt, EntityID entity ) {
		Player &player = this->vault->registry.get<Player>(playerEnt);
		Tile &tile = this->vault->registry.get<Tile>(entity);

		if (this->spendResources(playerEnt, player.resourceType, 10)) {			
			for (sf::Vector2i p : this->tileAround(tile, 1)) {
				if (!this->map->objs.get(p.x, p.y)) {
					this->vault->factory.createUnit(this->vault->registry, playerEnt, type, p.x, p.y);
					return true;
				}
			}
		}
		return false;
	}

};