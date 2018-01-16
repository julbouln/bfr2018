#pragma once

#include "GameSystem.hpp"

class MapLayersSystem : public GameSystem {
	std::map<std::string, std::vector<EntityID>> tiles;

	std::vector<EntityID> dirtTransitions;
	std::map<int, int> dirtTransitionsMapping;

public:
	void update(float dt) {
		this->updateTileLayer(dt);
		this->updateObjsLayer(dt);
		this->updatePlayersFog(dt);
	}

	void updateTileLayer(float dt) {
		auto view = this->vault->registry.persistent<Tile, Building, GameObject>();

		// update terrain with building
		for (EntityID entity : view) {
			Tile &tile = view.get<Tile>(entity);
			Building &building = view.get<Building>(entity);
			GameObject &obj = view.get<GameObject>(entity);

			if (obj.mapped) {
				for (sf::Vector2i p : this->tileSurfaceExtended(tile, 1)) {
					if (obj.team == "rebel") {
						this->map->terrains.set(p.x, p.y, tiles["grass"][0]);
					} else {
						this->map->terrains.set(p.x, p.y, tiles["concrete"][0]);
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
					this->map->terrains.set(p.x, p.y, tiles["grass"][0]);
				} else {
					this->map->terrains.set(p.x, p.y, tiles["concrete"][0]);
				}
			}

		}

		this->updateTransitions();
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

		std::sort( this->map->entities.begin( ), this->map->entities.end( ), [this ]( const auto & lhs, const auto & rhs )
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
			return (lht.pos.y + lht.size.y / 2 < rht.pos.y + rht.size.y / 2);
//			return (lht.ppos.y - (lht.centerRect.top + lht.centerRect.height / 2) + 16 + lht.offset.x*32 < rht.ppos.y - (rht.centerRect.top + rht.centerRect.height / 2) + + 16 + rht.offset.x*32);
		});
	}


	void updatePlayersFog(float dt) {

		auto playerView = this->vault->registry.view<Player>();
		for(EntityID entity : playerView) {
			Player &player = playerView.get(entity);

			for(int x=0;x<this->map->width;x++) {
				for(int y=0;y<this->map->height;y++) {
					if(player.fog.get(x,y) == FogState::InSight)
						player.fog.set(x,y,FogState::Hidden);
				}
			}
		}

		auto view = this->vault->registry.persistent<Tile, GameObject>();
		for (EntityID entity : view) {
			Tile &tile = view.get<Tile>(entity);
			GameObject &obj = view.get<GameObject>(entity);
			Player &player = this->vault->registry.get<Player>(obj.player);

			if(obj.mapped) {
			for (sf::Vector2i p : this->tileSurfaceExtended(tile, obj.view)) {
				player.fog.set(p.x, p.y, FogState::InSight);
			}
		}

		}
	}

// Terrains/Transitions

	std::vector<EntityID> initTile(std::string name) {
		std::vector<EntityID> tileVariants;
		for (int i = 0; i < 3; i++) {
			tileVariants.push_back(this->vault->factory.createTerrain(this->vault->registry, name, i));
		}
		return tileVariants;
	}

	EntityID randTile(std::string name) {
		int rnd = rand() % 3;
		return tiles[name][rnd];
	}

	void initTiles() {
		tiles["sand"] = this->initTile("sand");
		tiles["water"] = this->initTile("water");
		tiles["grass"] = this->initTile("grass");
		tiles["dirt"] = this->initTile("dirt");
		tiles["concrete"] = this->initTile("concrete");
	}

	void initTransitions() {
		for (int i = 0; i < 20; i++) {
			dirtTransitions.push_back(this->vault->factory.createTerrain(this->vault->registry, "dirt_transition", i));
		}

		dirtTransitionsMapping[1] = 2;
		dirtTransitionsMapping[2] = 0;
		dirtTransitionsMapping[3] = 4;
		dirtTransitionsMapping[4] = 1;
		dirtTransitionsMapping[5] = 6;
		dirtTransitionsMapping[8] = 3;
		dirtTransitionsMapping[10] = 7;
		dirtTransitionsMapping[12] = 5;

		// miss

	}

// https://gamedevelopment.tutsplus.com/tutorials/how-to-use-tile-bitmasking-to-auto-tile-your-level-layouts--cms-25673
	void updateTransitions() {
		for (int x = 0; x < this->map->width; x++) {
			for (int y = 0; y < this->map->height; y++) {
				int bitmask = 0;
				if (this->map->terrains.get(x, y) != tiles["dirt"][0]) {
					if (this->map->bound(x, y - 1))
						bitmask += 1 * ((this->map->terrains.get(x, y - 1) == tiles["dirt"][0]) ? 1 : 0);
					if (this->map->bound(x - 1, y))
						bitmask += 2 * ((this->map->terrains.get(x - 1, y) == tiles["dirt"][0]) ? 1 : 0);
					if (this->map->bound(x + 1, y))
						bitmask += 4 * ((this->map->terrains.get(x + 1, y) == tiles["dirt"][0]) ? 1 : 0);
					if (this->map->bound(x, y + 1))
						bitmask += 8 * ((this->map->terrains.get(x, y + 1) == tiles["dirt"][0]) ? 1 : 0);

//					if (bitmask)
//						std::cout << "BITMASK " << x << "x" << y << " : " << bitmask << std::endl;


				}

				if (bitmask) {
					int trans = 0;
					if (dirtTransitionsMapping.count(bitmask) > 0) {
						trans = dirtTransitions[dirtTransitionsMapping[bitmask]];
						this->map->transitions.set(x, y, trans);
					}
					else
						this->map->transitions.set(x, y, 0);
//						trans=dirtTransitions[bitmask];

				}
				else
					this->map->transitions.set(x, y, 0);

			}
		}
	}

	void generate(unsigned int width, unsigned int height) {
		this->map->terrains.width = width;
		this->map->terrains.height = height;

		this->map->transitions.width = width;
		this->map->transitions.height = height;

		this->map->objs.width = width;
		this->map->objs.height = height;

		this->map->resources.width = width;
		this->map->resources.height = height;

		this->map->width = width;
		this->map->height = height;

		float random_w = ((float) rand()) / (float) RAND_MAX;
		float random_h = ((float) rand()) / (float) RAND_MAX;

		SimplexNoise simpl(width / 16.0, height / 16.0, 2.0, 0.5);

		this->map->terrains.fill();
		this->map->transitions.fill();
		this->map->objs.fill();
		this->map->resources.fill();

		for (float y = 0; y < height; y++) {
			for (float x = 0; x < width; x++) {
				float res = (simpl.fractal(64, x / (width) + random_w * width, y / (height) + random_h * height));

				EntityID t;
//				t = this->randTile("dirt");
				t = tiles["dirt"][0];
				/*
								if (res > -0.5) {
									t = tiles["dirt"];
								} else {
									t = tiles["water"];
								}
				*/
				this->map->terrains.set(x, y, t);

				if (res > 0.6 && res < 0.65) {
					float rnd = ((float) rand()) / (float) RAND_MAX;
					if (rnd > 0.5) {
						this->vault->factory.plantResource(this->vault->registry, ResourceType::Nature, x, y);
					} else {
						this->vault->factory.plantResource(this->vault->registry, ResourceType::Pollution, x, y);
					}
				}
			}
		}


	}

};