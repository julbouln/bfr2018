#pragma once

#include "GameSystem.hpp"

class MapLayersSystem : public GameSystem {
	std::map<std::string, std::vector<EntityID>> tiles;

	std::vector<EntityID> dirtTransitions;
	std::vector<EntityID> waterTransitions;
	std::vector<EntityID> concreteTransitions;

	std::map<int, int> terrainTransitionsMapping;

	std::vector<EntityID> fogTransitions;

	std::map<int, int> fogTransitionsMapping;

	std::vector<EntityID> fogTransitions2;
	std::vector<EntityID> debugTransitions;

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

			for (sf::Vector2i p : this->tileSurfaceExtended(tile, 1)) {
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
		this->map->resources.clear();
		auto resView = this->vault->registry.persistent<Tile, Resource>();

		for (EntityID entity : resView) {
			Tile &tile = resView.get<Tile>(entity);

			for (sf::Vector2i p : this->tileSurface(tile)) {
				this->map->resources.set(p.x, p.y, entity);
			}
		}

		this->map->objs.clear();
		auto view = this->vault->registry.persistent<Tile, GameObject>();

		for (EntityID entity : view) {
			Tile &tile = view.get<Tile>(entity);

			for (sf::Vector2i p : this->tileSurface(tile)) {
				this->map->objs.set(p.x, p.y, entity);
			}
		}

/*
		auto unitView = this->vault->registry.persistent<Tile, Unit>();

		for (EntityID entity : unitView) {
			Tile &tile = unitView.get<Tile>(entity);
			Unit &unit = unitView.get<Unit>(entity);

			this->map->objs.set(unit.nextpos.x, unit.nextpos.y, entity);
		}
		*/
	}

	void updatePlayersFog(float dt) {
		auto playerView = this->vault->registry.view<Player>();
		for (EntityID entity : playerView) {
			Player &player = playerView.get(entity);

			for (int x = 0; x < this->map->width; x++) {
				for (int y = 0; y < this->map->height; y++) {
					if (player.fog.get(x, y) == FogState::InSight)
						player.fog.set(x, y, FogState::Hidden);
				}
			}
		}

		auto view = this->vault->registry.persistent<Tile, GameObject>();
		for (EntityID entity : view) {
			Tile &tile = view.get<Tile>(entity);
			GameObject &obj = view.get<GameObject>(entity);
			Player &player = this->vault->registry.get<Player>(obj.player);

			if (obj.mapped) {
				for (sf::Vector2i p : this->tileSurfaceExtended(tile, obj.view)) {
					player.fog.set(p.x, p.y, FogState::InSight);
				}
			}

		}

	}

	void updateFog(EntityID playerEnt, float dt) {
		Player &player = this->vault->registry.get<Player>(playerEnt);

		// draw debug grid
		for (int y = 0; y < this->map->height; ++y)
		{
			for (int x = 0; x < this->map->width; ++x)
			{
				FogState st = player.fog.get(x, y);

				if (st == FogState::Unvisited) {
					this->map->fog.set(x, y, fogTransitions[0]);
				} else {
					this->map->fog.set(x, y, 0);
				}

				if (st == FogState::Hidden) {
					this->map->fogHidden.set(x, y, fogTransitions[0]);
				} else {
					this->map->fogHidden.set(x, y, 0);
				}
			}
		}
		for (int y = 0; y < this->map->height; ++y)
		{
			for (int x = 0; x < this->map->width; ++x)
			{
				this->updateFogTransition(this->map->fog, x, y);
				this->updateFogTransition(this->map->fogHidden, x, y);
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

		for (int i = 0; i < 20; i++) {
			waterTransitions.push_back(this->vault->factory.createTerrain(this->vault->registry, "water_transition", i));
		}

		for (int i = 0; i < 20; i++) {
			concreteTransitions.push_back(this->vault->factory.createTerrain(this->vault->registry, "concrete_transition", i));
		}

		terrainTransitionsMapping[1] = 2;
		terrainTransitionsMapping[2] = 0;
		terrainTransitionsMapping[3] = 4;
		terrainTransitionsMapping[4] = 1;
		terrainTransitionsMapping[5] = 6;
		terrainTransitionsMapping[8] = 3;
		terrainTransitionsMapping[10] = 7;
		terrainTransitionsMapping[12] = 5;

		terrainTransitionsMapping[0x10] = 12;
		terrainTransitionsMapping[0x20] = 13;
		terrainTransitionsMapping[0x40] = 15;
		terrainTransitionsMapping[0x80] = 14;

		for (int i = 0; i < 13; i++) {
			fogTransitions.push_back(this->vault->factory.createTerrain(this->vault->registry, "fog_transition", i));
		}

		fogTransitionsMapping[1] = 3;
		fogTransitionsMapping[2] = 2;
		fogTransitionsMapping[3] = 5;
		fogTransitionsMapping[4] = 1;
		fogTransitionsMapping[5] = 6;
		fogTransitionsMapping[8] = 4;
		fogTransitionsMapping[10] = 8;
		fogTransitionsMapping[12] = 7;
		fogTransitionsMapping[15] = 0;

		fogTransitionsMapping[0x10] = 9;
		fogTransitionsMapping[0x20] = 10;
		fogTransitionsMapping[0x40] = 11;
		fogTransitionsMapping[0x80] = 12;

		for (int i = 0; i < 256; i++) {
			debugTransitions.push_back(this->vault->factory.createTerrain(this->vault->registry, "debug_transition", i));
		}
	}

// https://gamedevelopment.tutsplus.com/tutorials/how-to-use-tile-bitmasking-to-auto-tile-your-level-layouts--cms-25673
	int transitionBitmask(TileLayer &layer, EntityID ent, int x, int y) {
		int bitmask = 0;
		if (this->map->bound(x, y - 1))
			bitmask += 1 * ((layer.get(x, y - 1) == ent) ? 1 : 0);
		if (this->map->bound(x - 1, y))
			bitmask += 2 * ((layer.get(x - 1, y) == ent) ? 1 : 0);
		if (this->map->bound(x + 1, y))
			bitmask += 4 * ((layer.get(x + 1, y) == ent) ? 1 : 0);
		if (this->map->bound(x, y + 1))
			bitmask += 8 * ((layer.get(x, y + 1) == ent) ? 1 : 0);

		if (this->map->bound(x - 1, y - 1))
			bitmask += 16 * ((layer.get(x - 1, y - 1) == ent) ? 1 : 0);
		if (this->map->bound(x + 1, y - 1))
			bitmask += 32 * ((layer.get(x + 1, y - 1) == ent) ? 1 : 0);
		if (this->map->bound(x - 1, y + 1))
			bitmask += 64 * ((layer.get(x - 1, y + 1) == ent) ? 1 : 0);
		if (this->map->bound(x + 1, y + 1))
			bitmask += 128 * ((layer.get(x + 1, y + 1) == ent) ? 1 : 0);

		return bitmask;
	}

	int voidTransitionBitmask(TileLayer &layer, EntityID ent, int x, int y) {
		int bitmask = 0;
		if (layer.get(x, y) != ent) {
			bitmask = this->transitionBitmask(layer, ent, x, y);
		}
		return bitmask;
	}

	int pairTransitionBitmask(TileLayer &layer, EntityID srcEnt, EntityID dstEnt, int x, int y) {
		int bitmask = 0;
		if (layer.get(x, y) == srcEnt) {
			bitmask = this->transitionBitmask(layer, dstEnt, x, y);
		}
		return bitmask;
	}

	int updateTransition(int bitmask, TileLayer &outLayer, EntityID ent, std::vector<EntityID> &transitions, std::map<int, int> &mapping, int x, int y) {
		if (bitmask) {
			if (bitmask & 0xf) {
				if (mapping.count(bitmask & 0xf) > 0) {
					int trans = transitions[mapping[bitmask & 0xf]];
					outLayer.set(x, y, trans);
				} else {
					outLayer.set(x, y, debugTransitions[bitmask & 0xf]);

				}
			} else {
				if (mapping.count(bitmask) > 0) {
					int trans = transitions[mapping[bitmask]];
					outLayer.set(x, y, trans);
				} else {
					outLayer.set(x, y, debugTransitions[bitmask]);
				}
			}
		}
		return bitmask;
	}



	void updateGrassConcreteTransition(int x, int y) {
		EntityID grassEnt = tiles["grass"][0];
		EntityID concreteEnt = tiles["concrete"][0];
		int bitmask = this->pairTransitionBitmask(this->map->terrains, grassEnt, concreteEnt, x, y);

		bitmask = this->updateTransition(bitmask, this->map->transitions[0], concreteEnt, concreteTransitions, terrainTransitionsMapping, x, y);
		if(!bitmask) {
			this->map->transitions[0].set(x, y, 0);
		}
	}
	

	void updateSandWaterTransition(int x, int y) {
		EntityID sandEnt = tiles["sand"][0];
		EntityID waterEnt = tiles["water"][0];
		int bitmask = this->pairTransitionBitmask(this->map->terrains, sandEnt, waterEnt, x, y);

		bitmask = this->updateTransition(bitmask, this->map->transitions[1], waterEnt, waterTransitions, terrainTransitionsMapping, x, y);
		if(!bitmask) {
			this->map->transitions[1].set(x, y, 0);
		}
	}


	void updateDirtTransition(int x, int y) {
		EntityID dirtEnt = tiles["dirt"][0];
		int bitmask = this->voidTransitionBitmask(this->map->terrains, dirtEnt, x, y);
		bitmask = this->updateTransition(bitmask, this->map->transitions[2], dirtEnt, dirtTransitions, terrainTransitionsMapping, x, y);
		if(!bitmask) {
			this->map->transitions[2].set(x, y, 0);
		}
	}

	void updateFogTransition(TileLayer &layer, int x, int y) {
		EntityID fogEnt = fogTransitions[0];
		int bitmask = this->voidTransitionBitmask(layer, fogEnt, x, y);
		this->updateTransition(bitmask, layer, fogEnt, fogTransitions, fogTransitionsMapping, x, y);
	}

	void updateTransitions() {
		for (int x = 0; x < this->map->width; x++) {
			for (int y = 0; y < this->map->height; y++) {

				this->updateDirtTransition(x, y);
				this->updateGrassConcreteTransition(x, y);
				this->updateSandWaterTransition(x, y);
			}
		}
	}

	void generate(unsigned int width, unsigned int height) {
		this->map->setSize(width, height);

		float random_w = ((float) rand()) / (float) RAND_MAX;
		float random_h = ((float) rand()) / (float) RAND_MAX;

		SimplexNoise simpl(width / 64.0, height / 64.0, 2.0, 0.5);

		for (float y = 0; y < height; y++) {
			for (float x = 0; x < width; x++) {
				float res = (simpl.fractal(64, x / (width) + random_w * width, y / (height) + random_h * height));

				EntityID t;

				t = tiles["dirt"][0];
				if (res < -0.3)
					t = tiles["sand"][0];
				if (res < -0.5)
					t = tiles["water"][0];
				/*
								if (res > -0.4) {
									t = tiles["dirt"][0];
								} else {
									t = tiles["water"][0];
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