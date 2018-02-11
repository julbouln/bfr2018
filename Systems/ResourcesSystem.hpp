#pragma once

#include "GameSystem.hpp"

class ResourcesSystem : public GameSystem {
public:
	void update(float dt) {
		int natureResources = 0;
		int pollutionResources = 0;

		std::set<EntityID> markDelete;

		auto view = this->vault->registry.persistent<Tile, Resource>();
		for (EntityID entity : view) {
			Resource &resource = view.get<Resource>(entity);
			Tile &tile = view.get<Tile>(entity);
			resource.grow += resource.growRate;

			if (resource.grow > resource.maxGrow) {
				resource.grow = 0.0;

				if (resource.level < resource.maxLevel) {
					resource.level++;
					if (resource.level == 1) {
						this->vault->factory.growedResource(this->vault->registry, resource.type, entity);
						Tile &newTile = this->vault->registry.get<Tile>(entity);
						for (sf::Vector2i &p : this->tileSurface(newTile)) {
							EntityID posEnt = this->map->resources.get(p.x, p.y);
							if (posEnt != 0 && posEnt != entity) {
								markDelete.insert(posEnt);
								this->map->resources.set(p.x, p.y, 0);
							}
						}
					}
					else {
						tile.view = resource.level - 1;
					}
				} else {
					// max
					this->seedResources(resource.type, entity);
					markDelete.insert(entity);
				}
			}

		}

		for (EntityID delEnt : markDelete) {
			if (this->vault->registry.valid(delEnt))
				this->vault->registry.destroy(delEnt);
		}

		auto resView = this->vault->registry.view<Resource>();
		for (EntityID entity : resView) {
			Resource &resource = resView.get(entity);
			if (resource.type == "nature")
				natureResources += resource.level;
			else
				pollutionResources += resource.level;

		}

		auto playerView = this->vault->registry.view<Player>();
		for (EntityID entity : playerView) {
			Player &player = playerView.get(entity);
			if (player.team == "rebel")
				player.resources = natureResources;
			else if (player.team == "neonaz")
				player.resources = pollutionResources;
		}
	}
};