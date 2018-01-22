#pragma once

#include "GameSystem.hpp"

//#define TRANSITIONS_DEBUG

#define ALT_TILES 3

struct CompareVector2i
{
	bool operator()(sf::Vector2i a, sf::Vector2i b) const
	{
		if (a.x < b.x)
			return true;
		else if (b.x < a.x)
			return false;
		else
			return a.y < b.y;
	}
};

class MapLayersSystem : public GameSystem {
	std::map<std::string, std::vector<EntityID>> tiles;

	std::vector<EntityID> dirtTransitions;
	std::vector<EntityID> waterTransitions;
	std::vector<EntityID> concreteTransitions;

	std::map<int, int> terrainTransitionsMapping;

	std::vector<EntityID> fogTransitions;

	std::map<int, int> fogTransitionsMapping;

	std::vector<EntityID> debugTransitions;

	// transitions calculation optimization
	// maintain a list of position to update instead of updating every transitions

	std::set<sf::Vector2i, CompareVector2i> markUpdateTerrainTransitions;
	std::set<sf::Vector2i, CompareVector2i> markUpdateFogTransitions;

	std::map<EntityID, EntityID> altTerrains;

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
				// delete resources under building
				for (sf::Vector2i p : this->tileSurface(tile)) {
					EntityID resEnt = this->map->resources.get(p.x, p.y);
					if (resEnt && this->vault->registry.valid(resEnt)) {
						this->vault->registry.destroy(resEnt);
					}
				}

				for (sf::Vector2i p : this->tileSurfaceExtended(tile, 1)) {
					EntityID newEnt = 0;
					if (obj.team == "rebel") {
						newEnt = tiles["grass"][0];
					} else {
						newEnt = tiles["concrete"][0];
					}

					if (this->altTerrains[this->map->terrains.get(p.x, p.y)] != this->altTerrains[newEnt]) {
						for (sf::Vector2i sp : this->vectorSurfaceExtended(p, 1))
							this->markUpdateTerrainTransitions.insert(sp);

						this->map->terrains.set(p.x, p.y, newEnt);
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
				EntityID newEnt = 0;
				if (resource.type == ResourceType::Nature) {
					newEnt = tiles["grass"][0];
				} else {
					newEnt = tiles["concrete"][0];
				}

				if (this->altTerrains[this->map->terrains.get(p.x, p.y)] != this->altTerrains[newEnt]) {
					for (sf::Vector2i sp : this->vectorSurfaceExtended(p, 1))
						this->markUpdateTerrainTransitions.insert(sp);
					this->map->terrains.set(p.x, p.y, newEnt);
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

	void updateFogLayer(EntityID playerEnt, float dt) {
		Player &player = this->vault->registry.get<Player>(playerEnt);

		for (int y = 0; y < this->map->height; ++y)
		{
			for (int x = 0; x < this->map->width; ++x)
			{
				sf::Vector2i p = sf::Vector2i(x, y);
				FogState st = player.fog.get(x, y);
				bool markUpdate = false;
				EntityID newEnt = 0;

				if (st == FogState::Unvisited) {
					newEnt = fogTransitions[0];
				} else {
					newEnt = 0;
				}

				if (this->map->fogUnvisited.get(x, y) != newEnt)
				{
					markUpdate = true;
				}

				this->map->fogUnvisited.set(x, y, newEnt);

				if (st == FogState::Hidden) {
					newEnt = fogTransitions[0];
				} else {
					newEnt = 0;
				}

				if (this->map->fogHidden.get(x, y) != newEnt) {
					markUpdate = true;
				}

					this->map->fogHidden.set(x, y, newEnt);

				if (markUpdate) {
					for (sf::Vector2i sp : this->vectorSurfaceExtended(p, 1))
						this->markUpdateFogTransitions.insert(sp);
				}
			}
		}

#ifdef TRANSITIONS_DEBUG
		std::cout << "Transitions: update " << this->markUpdateFogTransitions.size() << " FOG transitions" << std::endl;
#endif

		for (sf::Vector2i p : this->markUpdateFogTransitions) {
			this->updateFogHiddenTransition(p.x, p.y);
			this->updateFogUnvisitedTransition(p.x, p.y);
		}

		this->markUpdateFogTransitions.clear();
	}

	EntityID getTile(std::string name, int n) {
		return tiles[name][n];
	}

// Terrains/Transitions

	std::vector<EntityID> initTerrain(std::string name) {
		std::vector<EntityID> tileVariants;
		EntityID origTerrain = this->vault->factory.createTerrain(this->vault->registry, name, 0);
		tileVariants.push_back(origTerrain);
		this->altTerrains[origTerrain] = origTerrain;

		for (int i = 1; i < 3; i++) {
			EntityID altTerrain = this->vault->factory.createTerrain(this->vault->registry, name, i);
			tileVariants.push_back(altTerrain);
			this->altTerrains[altTerrain] = origTerrain;
		}
		return tileVariants;
	}

	EntityID randTerrain(std::string name) {
		int rnd = rand() % ALT_TILES;
		return tiles[name][rnd];
	}

	void initTerrains() {
		tiles["sand"] = this->initTerrain("sand");
		tiles["water"] = this->initTerrain("water");
		tiles["grass"] = this->initTerrain("grass");
		tiles["dirt"] = this->initTerrain("dirt");
		tiles["concrete"] = this->initTerrain("concrete");
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

		altTerrains[fogTransitions[0]] = fogTransitions[0];

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

	void initCorpse(std::string name) {
		Tile tile;
		this->vault->factory.parseTileFromXml(name, tile, 8);


		tile.pos = sf::Vector2i(0, 0);
		tile.ppos = sf::Vector2f(tile.pos) * (float)32.0;

		tile.sprite.setTexture(this->vault->factory.getTex(name));

		tile.state = "die";

		AnimationHandler &dieAnim = tile.animHandlers["die"];

		dieAnim.changeAnim(0);
		int frame = dieAnim.getAnim().getFrame(dieAnim.getAnim().getLength() - 1);
//		std::cout << "CORPSE frame "<<name << " "<<frame<<std::endl;
		dieAnim.set(frame);

		tile.sprite.setTextureRect(dieAnim.bounds); // texture need to be updated

		tile.centerRect = this->vault->factory.getCenterRect(name);

		std::vector<EntityID> tvec;
		tvec.push_back(this->vault->registry.create());
		this->vault->registry.assign<Tile>(tvec.front(), tile);
		tiles[name + "_corpse"] = tvec;
	}

	void initCorpses() {
		for (TechNode *node : this->vault->factory.getTechNodes("rebel")) {
			if (node->comp == TechComponent::Character) {
				this->initCorpse(node->type);
			}
		}
		for (TechNode *node : this->vault->factory.getTechNodes("neonaz")) {
			if (node->comp == TechComponent::Character) {
				this->initCorpse(node->type);
			}
		}

		Tile ruinTile;
		ruinTile.pos = sf::Vector2i(0, 0);
		ruinTile.ppos = sf::Vector2f(ruinTile.pos) * (float)32.0;

		ruinTile.sprite.setTexture(this->vault->factory.getTex("ruin"));

		ruinTile.centerRect = this->vault->factory.getCenterRect("ruin");

		Animation staticAnim({0});

		AnimationHandler idleHandler;

		idleHandler.frameSize = sf::IntRect(0, 0, 192, 128);

		idleHandler.addAnim(staticAnim);

		idleHandler.changeAnim(0);
		idleHandler.set(0);

		ruinTile.sprite.setTextureRect(idleHandler.bounds); // texture need to be updated

		ruinTile.animHandlers["idle"] = idleHandler;

		ruinTile.direction = North;
		ruinTile.state = "idle";

		std::vector<EntityID> tvec;
		tvec.push_back(this->vault->registry.create());
		this->vault->registry.assign<Tile>(tvec.front(), ruinTile);
		tiles["ruin"] = tvec;

	}

// https://gamedevelopment.tutsplus.com/tutorials/how-to-use-tile-bitmasking-to-auto-tile-your-level-layouts--cms-25673
	int transitionBitmask(TileLayer &layer, EntityID ent, int x, int y) {
		int bitmask = 0;
		if (this->map->bound(x, y - 1))
			bitmask += 1 * ((this->altTerrains[layer.get(x, y - 1)] == ent) ? 1 : 0);
		if (this->map->bound(x - 1, y))
			bitmask += 2 * ((this->altTerrains[layer.get(x - 1, y)] == ent) ? 1 : 0);
		if (this->map->bound(x + 1, y))
			bitmask += 4 * ((this->altTerrains[layer.get(x + 1, y)] == ent) ? 1 : 0);
		if (this->map->bound(x, y + 1))
			bitmask += 8 * ((this->altTerrains[layer.get(x, y + 1)] == ent) ? 1 : 0);

		if (this->map->bound(x - 1, y - 1))
			bitmask += 16 * ((this->altTerrains[layer.get(x - 1, y - 1)] == ent) ? 1 : 0);
		if (this->map->bound(x + 1, y - 1))
			bitmask += 32 * ((this->altTerrains[layer.get(x + 1, y - 1)] == ent) ? 1 : 0);
		if (this->map->bound(x - 1, y + 1))
			bitmask += 64 * ((this->altTerrains[layer.get(x - 1, y + 1)] == ent) ? 1 : 0);
		if (this->map->bound(x + 1, y + 1))
			bitmask += 128 * ((this->altTerrains[layer.get(x + 1, y + 1)] == ent) ? 1 : 0);

		return bitmask;
	}

	int voidTransitionBitmask(TileLayer &layer, EntityID ent, int x, int y) {
		int bitmask = 0;
		if (this->altTerrains[layer.get(x, y)] != ent) {
			bitmask = this->transitionBitmask(layer, ent, x, y);
		}
		return bitmask;
	}

	int pairTransitionBitmask(TileLayer &layer, EntityID srcEnt, EntityID dstEnt, int x, int y) {
		int bitmask = 0;
		if (this->altTerrains[layer.get(x, y)] == srcEnt) {
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
		if (!bitmask) {
			this->map->transitions[0].set(x, y, 0);
		}
	}


	void updateSandWaterTransition(int x, int y) {
		EntityID sandEnt = tiles["sand"][0];
		EntityID waterEnt = tiles["water"][0];
		int bitmask = this->pairTransitionBitmask(this->map->terrains, sandEnt, waterEnt, x, y);

		bitmask = this->updateTransition(bitmask, this->map->transitions[1], waterEnt, waterTransitions, terrainTransitionsMapping, x, y);
		if (!bitmask) {
			this->map->transitions[1].set(x, y, 0);
		}
	}


	void updateDirtTransition(int x, int y) {
		EntityID dirtEnt = tiles["dirt"][0];
		int bitmask = this->voidTransitionBitmask(this->map->terrains, dirtEnt, x, y);
		bitmask = this->updateTransition(bitmask, this->map->transitions[2], dirtEnt, dirtTransitions, terrainTransitionsMapping, x, y);
		if (!bitmask) {
			this->map->transitions[2].set(x, y, 0);
		}
	}

	void updateAllTransitions() {
		for (int x = 0; x < this->map->width; x++) {
			for (int y = 0; y < this->map->height; y++) {
				this->updateDirtTransition(x, y);
				this->updateGrassConcreteTransition(x, y);
				this->updateSandWaterTransition(x, y);
			}
		}
	}

	void updateTransitions() {
#ifdef TRANSITIONS_DEBUG
		std::cout << "Transitions: update " << this->markUpdateTerrainTransitions.size() << " terrain transitions" << std::endl;
#endif

		for (sf::Vector2i p : this->markUpdateTerrainTransitions) {
			this->updateDirtTransition(p.x, p.y);
			this->updateGrassConcreteTransition(p.x, p.y);
			this->updateSandWaterTransition(p.x, p.y);
		}
		this->markUpdateTerrainTransitions.clear();
	}

	// FOG transition

	void updateFogUnvisitedTransition(int x, int y) {
		EntityID fogEnt = fogTransitions[0];
		int bitmask = this->voidTransitionBitmask(this->map->fogUnvisited, fogEnt, x, y);
		bitmask = this->updateTransition(bitmask, this->map->fogUnvisitedTransitions, fogEnt, fogTransitions, fogTransitionsMapping, x, y);
		if (!bitmask) {
			this->map->fogUnvisitedTransitions.set(x, y, 0);
		}
	}

	void updateFogHiddenTransition(int x, int y) {
		EntityID fogEnt = fogTransitions[0];
		int bitmask = this->voidTransitionBitmask(this->map->fogHidden, fogEnt, x, y);
		bitmask = this->updateTransition(bitmask, this->map->fogHiddenTransitions, fogEnt, fogTransitions, fogTransitionsMapping, x, y);
		if (!bitmask) {
			this->map->fogHiddenTransitions.set(x, y, 0);
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

				t = tiles["dirt"][rand() % ALT_TILES];
				if (res < -0.3)
					t = tiles["sand"][rand() % ALT_TILES];
				if (res < -0.5)
					t = tiles["water"][rand() % ALT_TILES];
				/*
								if (res > -0.4) {
									t = tiles["dirt"][0];
								} else {
									t = tiles["water"][0];
								}

				*/
				this->map->terrains.set(x, y, t);
				/*
								if (res > 0.6 && res < 0.65) {
									float rnd = ((float) rand()) / (float) RAND_MAX;
									if (rnd > 0.5) {
										this->vault->factory.plantResource(this->vault->registry, ResourceType::Nature, x, y);
									} else {
										this->vault->factory.plantResource(this->vault->registry, ResourceType::Pollution, x, y);
									}
								}
								*/
			}
		}

		this->updateAllTransitions();
	}

};