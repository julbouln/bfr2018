#pragma once

#include "GameSystem.hpp"


class ResourcesSystem : public GameSystem {
public:
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

	void update(float dt) {
		int natureResources = 0;
		int pollutionResources = 0;

		auto view = this->vault->registry.persistent<Tile, Resource>();
		for (EntityID entity : view) {
			Resource &resource = view.get<Resource>(entity);
			Tile &tile = view.get<Tile>(entity);
			resource.grow += 0.1;

//				std::cout << "RESOURCE "<<entity<<" grow "<<resource.grow << std::endl;

			if (resource.grow > 10) {
				std::cout << "RESOURCE " << entity << " grow" << std::endl;
				resource.grow = 0.0;

				if (resource.level < 3) {
					resource.level++;
					if (resource.level == 1)
						this->vault->factory.growedResource(this->vault->registry, this->vault->factory.resourceTypeName(resource.type), entity);
					else
						tile.animHandlers[tile.state].set(resource.level - 1);
				} else {
					// max
					this->seedResources(resource.type, entity);

				}
			}

			switch (resource.type) {
			case ResourceType::Nature:
				natureResources += resource.level;
				break;
			case ResourceType::Pollution:
				pollutionResources += resource.level;
				break;
			}

		}

		auto playerView = this->vault->registry.view<Player>();
		for (EntityID entity : playerView) {
			Player &player = playerView.get(entity);
			if (player.team == "rebel")
				player.resources = natureResources;
			else
				player.resources = pollutionResources;
		}
	}
};