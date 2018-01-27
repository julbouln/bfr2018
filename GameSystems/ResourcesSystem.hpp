#pragma once

#include "GameSystem.hpp"

class ResourcesSystem : public GameSystem {
public:

	void update(float dt) {
		int natureResources = 0;
		int pollutionResources = 0;

		auto view = this->vault->registry.persistent<Tile, Resource>();
		for (EntityID entity : view) {
			Resource &resource = view.get<Resource>(entity);
			Tile &tile = view.get<Tile>(entity);
			resource.grow += 0.1;

			if (resource.grow > 10) {
//				std::cout << "RESOURCE " << entity << " grow" << std::endl;
				resource.grow = 0.0;

				if (resource.level < 3) {
					resource.level++;
					if (resource.level == 1) {
						this->vault->factory.growedResource(this->vault->registry, resource.type, entity);
						Tile &newTile = this->vault->registry.get<Tile>(entity);
//						std::cout << "NEW RESOURCE "<<tile.animHandlers[tile.state].bounds.left << "x" << tile.animHandlers[tile.state].bounds.top << ":" << tile.animHandlers[tile.state].bounds.width << "x" << tile.animHandlers[tile.state].bounds.height << std::endl;
					}
					else {
						tile.animHandlers[tile.state].set(resource.level - 1);
					}
				} else {
					// max
					this->seedResources(resource.type, entity);
//					this->vault->registry.destroy(entity);
				}
			}

			if(resource.type == "nature")
				natureResources += resource.level;
			else
				pollutionResources += resource.level;
		}

		auto playerView = this->vault->registry.view<Player>();
		for (EntityID entity : playerView) {
			Player &player = playerView.get(entity);
			if (player.team == "rebel")
				player.resources = natureResources;
			else if(player.team == "neonaz")
				player.resources = pollutionResources;
		}
	}
};