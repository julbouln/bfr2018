#pragma once

#include "GameSystem.hpp"

class MapLayersSystem : public GameSystem {
public:
	void update(float dt) {
		this->updateTileLayer(dt);
		this->updateObjsLayer(dt);
	}

	void updateTileLayer(float dt) {
		auto view = this->vault->registry.persistent<Tile, Building, GameObject>();

		// update tile with building
		for (EntityID entity : view) {
			Tile &tile = view.get<Tile>(entity);
			Building &building = view.get<Building>(entity);
			GameObject &obj = view.get<GameObject>(entity);

			if (building.built) {
				for (sf::Vector2i p : this->tileSurfaceExtended(tile)) {
					if (obj.team == "rebel") {
						this->map->terrains.set(p.x, p.y, this->map->tiles["grass"][0]);
					} else {
						this->map->terrains.set(p.x, p.y, this->map->tiles["concrete"][0]);
					}
				}
			}
		}

		// update tile with resource
		auto resView = this->vault->registry.persistent<Tile, Resource>();
		for (EntityID entity : resView) {
			Tile &tile = resView.get<Tile>(entity);
			Resource &resource = resView.get<Resource>(entity);

			for (sf::Vector2i p : this->tileSurface(tile)) {
				if (resource.type == ResourceType::Nature) {
					this->map->terrains.set(p.x, p.y, this->map->tiles["grass"][0]);
				} else {
					this->map->terrains.set(p.x, p.y, this->map->tiles["concrete"][0]);
				}
			}

		}

		this->map->updateTransitions();

	}

	void updateObjsLayer(float dt) {
		this->map->clearEntities();

		this->map->resources.clear();
		auto resView = this->vault->registry.persistent<Tile, Resource>();

		for (EntityID entity : resView) {
			Tile &tile = resView.get<Tile>(entity);

			for (sf::Vector2i p : this->tileSurface(tile)) {
				this->map->resources.set(p.x, p.y, entity);
			}

			this->map->addEntity(entity);
		}

		this->map->objs.clear();
		auto view = this->vault->registry.persistent<Tile, GameObject>();

		for (EntityID entity : view) {
			Tile &tile = view.get<Tile>(entity);

			for (sf::Vector2i p : this->tileSurface(tile)) {
				this->map->objs.set(p.x, p.y, entity);
			}

			this->map->addEntity(entity);
		}

		auto unitView = this->vault->registry.persistent<Tile, Unit>();

		for (EntityID entity : unitView) {
			Tile &tile = unitView.get<Tile>(entity);
			Unit &unit = unitView.get<Unit>(entity);

			this->map->objs.set(unit.nextpos.x, unit.nextpos.y, entity);
		}

		std::sort( this->map->entities.begin( ),this->map->entities.end( ), [this ]( const auto & lhs, const auto & rhs )
		{
			Tile &lht = vault->registry.get<Tile>(lhs);
			Tile &rht = vault->registry.get<Tile>(rhs);
			/*			if (lht.pos.y < rht.pos.y)
							return true;
						else if (lht.pos.y == rht.pos.y)
							return lht.pos.x < rht.pos.x;
						else
							return false;
			*/
			return (lht.pos.y < rht.pos.y);
		});
	}
};