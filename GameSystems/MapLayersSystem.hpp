#pragma once

#include "GameSystem.hpp"

#define ALT_TILES 3

class MapLayersSystem : public GameSystem {
	std::map<std::string, std::vector<EntityID>> tiles;

	std::vector<EntityID> dirtTransitions;
	std::vector<EntityID> waterTransitions;
	std::vector<EntityID> sandTransitions;
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
		this->updateLayer(dt);
		this->updateObjsLayer(dt);
		this->updatePlayersFog(dt);
	}

	void updateLayer(float dt) {
		auto view = this->vault->registry.persistent<Tile, Building, GameObject>();

		// update terrain with building
		for (EntityID entity : view) {
			Tile &tile = view.get<Tile>(entity);
			Building &building = view.get<Building>(entity);
			GameObject &obj = view.get<GameObject>(entity);

			if (obj.mapped) {
				// delete resources under building
				for (sf::Vector2i const &p : this->tileSurface(tile)) {
					EntityID resEnt = this->map->resources.get(p.x, p.y);
					if (resEnt && this->vault->registry.valid(resEnt)) {
						this->vault->registry.destroy(resEnt);
					}
				}

				for (sf::Vector2i const &p : this->tileSurfaceExtended(tile, 1)) {
					EntityID newEnt = 0;
					if (obj.team == "rebel") {
						newEnt = tiles["grass"][0];
					} else if (obj.team == "neonaz") {
						newEnt = tiles["concrete"][0];
					}

					if (this->map->terrainsForTransitions.get(p.x, p.y) != newEnt) {
						for (sf::Vector2i const &sp : this->vectorSurfaceExtended(p, 1)) {
							this->markUpdateTerrainTransitions.insert(sp);
						}

						if (this->map->staticBuildable.get(p.x, p.y) == 0) {
							this->map->terrains.set(p.x, p.y, newEnt);
							this->map->terrainsForTransitions.set(p.x, p.y, newEnt);
						}
					}

				}
			}
		}

		// update tile with resource
		auto resView = this->vault->registry.persistent<Tile, Resource>();
		for (EntityID entity : resView) {
			Tile &tile = resView.get<Tile>(entity);
			Resource &resource = resView.get<Resource>(entity);

			for (sf::Vector2i const &p : this->tileSurfaceExtended(tile, 1)) {
				EntityID newEnt = 0;
				if (resource.type == "nature") {
					newEnt = tiles["grass"][0];
				} else {
					newEnt = tiles["concrete"][0];
				}

				if (this->map->terrainsForTransitions.get(p.x, p.y) != newEnt) {
					for (sf::Vector2i const &sp : this->vectorSurfaceExtended(p, 1)) {
						this->markUpdateTerrainTransitions.insert(sp);
					}

					if (this->map->staticBuildable.get(p.x, p.y) == 0) {
						this->map->terrains.set(p.x, p.y, newEnt);
						this->map->terrainsForTransitions.set(p.x, p.y, newEnt);
					}
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

			for (sf::Vector2i const &p : this->tileSurface(tile)) {
				this->map->resources.set(p.x, p.y, entity);
			}
		}

		this->map->objs.clear();
		auto view = this->vault->registry.persistent<Tile, GameObject>();

		for (EntityID entity : view) {
			Tile &tile = view.get<Tile>(entity);

			for (sf::Vector2i const &p : this->tileSurface(tile)) {
				this->map->objs.set(p.x, p.y, entity);
			}
		}
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
				for (sf::Vector2i const &p : this->tileSurfaceExtended(tile, obj.view)) {
					player.fog.set(p.x, p.y, FogState::InSight);
				}
			}

		}
	}

	// spectator FOG concat all other players fog
	void updateSpectatorFog(EntityID playerEnt, float dt) {
		Player &player = this->vault->registry.get<Player>(playerEnt);

		if (player.team == "neutral") {
			auto playerView = this->vault->registry.view<Player>();
			for (int y = 0; y < this->map->height; ++y) {
				for (int x = 0; x < this->map->width; ++x) {
					player.fog.set(x, y, FogState::InSight);
				}
			}
		}
		/*
		if (player.team == "neutral") {
			auto playerView = this->vault->registry.view<Player>();
			for (EntityID entity : playerView) {
				if (entity != playerEnt) {
					Player &otherPlayer = playerView.get(entity);
					for (int y = 0; y < this->map->height; ++y) {
						for (int x = 0; x < this->map->width; ++x) {
							if (player.fog.get(x, y) == FogState::Hidden && otherPlayer.fog.get(x, y) != FogState::Unvisited) {
								player.fog.set(x, y, otherPlayer.fog.get(x, y));
							}
							if (player.fog.get(x, y) == FogState::Unvisited) {
								player.fog.set(x, y, otherPlayer.fog.get(x, y));
							}
						}
					}
				}
			}
		}
		*/
	}

	void updatePlayerFogLayer(EntityID playerEnt, sf::IntRect clip, float dt) {
		Player &player = this->vault->registry.get<Player>(playerEnt);

		for (int y = clip.top; y < clip.top + clip.height; ++y) {
			for (int x = clip.left; x < clip.left + clip.width; ++x) {
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
					for (sf::Vector2i const &sp : this->vectorSurfaceExtended(p, 1)) {
						this->markUpdateFogTransitions.insert(sp);
					}
				}
			}
		}

#ifdef TRANSITIONS_DEBUG
		std::cout << "Transitions: update " << this->markUpdateFogTransitions.size() << " FOG transitions" << std::endl;
#endif

		for (sf::Vector2i const &p : this->markUpdateFogTransitions) {
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
		for (int i = 0; i < 32; i++) {
			dirtTransitions.push_back(this->vault->factory.createTerrain(this->vault->registry, "dirt_transition", i));
			waterTransitions.push_back(this->vault->factory.createTerrain(this->vault->registry, "water_transition", i));
			sandTransitions.push_back(this->vault->factory.createTerrain(this->vault->registry, "sand_transition", i));
			concreteTransitions.push_back(this->vault->factory.createTerrain(this->vault->registry, "concrete_transition", i));
		}
		/*
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
		*/

		terrainTransitionsMapping[1] = 1;
		terrainTransitionsMapping[2] = 2;
		terrainTransitionsMapping[3] = 3;
		terrainTransitionsMapping[4] = 4;
		terrainTransitionsMapping[5] = 5;
		terrainTransitionsMapping[8] = 8;
		terrainTransitionsMapping[10] = 10;
		terrainTransitionsMapping[12] = 12;

		terrainTransitionsMapping[0x10] = 16 + 1;
		terrainTransitionsMapping[0x20] = 16 + 2;
		terrainTransitionsMapping[0x40] = 16 + 4;
		terrainTransitionsMapping[0x80] = 16 + 8;

		// auto generated
		terrainTransitionsMapping[6] = 6;
		terrainTransitionsMapping[7] = 7;
		terrainTransitionsMapping[9] = 9;
		terrainTransitionsMapping[11] = 11;
		terrainTransitionsMapping[13] = 13;
		terrainTransitionsMapping[14] = 14;
		terrainTransitionsMapping[15] = 15;

		terrainTransitionsMapping[0x30] = 16 + 3;
		terrainTransitionsMapping[0x50] = 16 + 5;

		terrainTransitionsMapping[0xa0] = 16 + 10;
		terrainTransitionsMapping[0xc0] = 16 + 12;
		terrainTransitionsMapping[0xd0] = 16 + 13;
		terrainTransitionsMapping[0xe0] = 16 + 14;
		terrainTransitionsMapping[0xf0] = 16 + 15;

//		terrainTransitionsMapping[6] = 16; // = 2+4
		// 7 2+4+1
//		terrainTransitionsMapping[9] = 17; // = 1+8
		// 11 1+8+2
		// 13 1+8+4
		// 14 2+4+8
		// 15 2+4+1+8


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

	void initCorpse(std::string name, EntityID playerEnt) {
		Tile tile;
		this->vault->factory.parseTileFromXml(name, tile);

		tile.pos = sf::Vector2i(0, 0);
		tile.ppos = sf::Vector2f(tile.pos) * (float)32.0;
		tile.z = 0;

		tile.sprite.setTexture(this->vault->factory.getTex(name));
		tile.state = "die";
		tile.shader = false;
		this->vault->factory.setColorSwapShader(this->vault->registry, tile, playerEnt);

		AnimationHandler &dieAnim = tile.animHandlers["die"];

		dieAnim.changeColumn(0);
		int frame = dieAnim.getAnim().getFrame(dieAnim.getAnim().getLength() - 1);
		dieAnim.getAnim().repeat = false;
		dieAnim.set(frame);

		tile.sprite.setTextureRect(dieAnim.bounds); // texture need to be updated

		tile.centerRect = this->vault->factory.getCenterRect(name);

		std::vector<EntityID> tvec;
		tvec.push_back(this->vault->registry.create());
		this->vault->registry.assign<Tile>(tvec.front(), tile);
		tiles[name + "_corpse_" + std::to_string(playerEnt)] = tvec; // UGLY and unoptimized
	}

	// init corpses and ruins tiles, must be called after player creation
	void initCorpses() {
		auto playerView = this->vault->registry.view<Player>();
		for (EntityID playerEnt : playerView) {
			Player &player = playerView.get(playerEnt);
			for (TechNode *node : this->vault->factory.getTechNodes(player.team)) {
				if (node->comp == TechComponent::Character) {
					this->initCorpse(node->type, playerEnt);
				}
			}
		}

		Tile ruinTile;
		ruinTile.pos = sf::Vector2i(0, 0);
		ruinTile.ppos = sf::Vector2f(ruinTile.pos) * (float)32.0;
		ruinTile.shader = false;

		ruinTile.sprite.setTexture(this->vault->factory.getTex("ruin"));

		ruinTile.centerRect = this->vault->factory.getCenterRect("ruin");

		Animation staticAnim({0});

		AnimationHandler idleHandler;

		idleHandler.frameSize = sf::IntRect(0, 0, 192, 128);

		idleHandler.addAnim(staticAnim);

		idleHandler.changeColumn(0);
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
	int transitionBitmask(Layer & layer, EntityID ent, int x, int y) {
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

	int voidTransitionBitmask(Layer & layer, EntityID ent, int x, int y) {
		int bitmask = 0;
		if (layer.get(x, y) != ent) {
			bitmask = this->transitionBitmask(layer, ent, x, y);
		}
		return bitmask;
	}

	int pairTransitionBitmask(Layer & layer, EntityID srcEnt, EntityID dstEnt, int x, int y) {
		int bitmask = 0;
		if (layer.get(x, y) == srcEnt) {
			bitmask = this->transitionBitmask(layer, dstEnt, x, y);
		}
		return bitmask;
	}

	int updateTransition(int bitmask, Layer & outLayer, EntityID ent, std::vector<EntityID> &transitions, std::map<int, int> &mapping, int x, int y) {
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
		int bitmask = this->pairTransitionBitmask(this->map->terrainsForTransitions, grassEnt, concreteEnt, x, y);

		bitmask = this->updateTransition(bitmask, this->map->transitions[0], concreteEnt, concreteTransitions, terrainTransitionsMapping, x, y);
		if (!bitmask) {
			this->map->transitions[0].set(x, y, 0);
		}
	}

	void updateSandWaterTransition(int x, int y) {
		EntityID sandEnt = tiles["sand"][0];
		EntityID waterEnt = tiles["water"][0];
		int bitmask = this->pairTransitionBitmask(this->map->terrainsForTransitions, sandEnt, waterEnt, x, y);

		bitmask = this->updateTransition(bitmask, this->map->transitions[1], waterEnt, waterTransitions, terrainTransitionsMapping, x, y);
		if (!bitmask) {
			this->map->transitions[1].set(x, y, 0);
		}
	}

	void updateGrassSandTransition(int x, int y) {
		EntityID grassEnt = tiles["grass"][0];
		EntityID sandEnt = tiles["sand"][0];
		int bitmask = this->pairTransitionBitmask(this->map->terrainsForTransitions, grassEnt, sandEnt, x, y);

		bitmask = this->updateTransition(bitmask, this->map->transitions[2], grassEnt, sandTransitions, terrainTransitionsMapping, x, y);
		if (!bitmask) {
			this->map->transitions[2].set(x, y, 0);
		}
	}

	void updateConcreteSandTransition(int x, int y) {
		EntityID concEnt = tiles["concrete"][0];
		EntityID sandEnt = tiles["sand"][0];
		int bitmask = this->pairTransitionBitmask(this->map->terrainsForTransitions, concEnt, sandEnt, x, y);

		bitmask = this->updateTransition(bitmask, this->map->transitions[3], sandEnt, sandTransitions, terrainTransitionsMapping, x, y);
		if (!bitmask) {
			this->map->transitions[3].set(x, y, 0);
		}
	}

	void updateDirtTransition(int x, int y) {
		EntityID dirtEnt = tiles["dirt"][0];
		int bitmask = this->voidTransitionBitmask(this->map->terrainsForTransitions, dirtEnt, x, y);
		bitmask = this->updateTransition(bitmask, this->map->transitions[4], dirtEnt, dirtTransitions, terrainTransitionsMapping, x, y);
		if (!bitmask) {
			this->map->transitions[4].set(x, y, 0);
		}
	}

	void updateAllTransitions() {
		for (int x = 0; x < this->map->width; x++) {
			for (int y = 0; y < this->map->height; y++) {
				this->updateDirtTransition(x, y);
				this->updateGrassConcreteTransition(x, y);
				this->updateSandWaterTransition(x, y);
				this->updateGrassSandTransition(x, y);
				this->updateConcreteSandTransition(x, y);
			}
		}
	}

	void updateTransitions() {
#ifdef TRANSITIONS_DEBUG
		std::cout << "Transitions: update " << this->markUpdateTerrainTransitions.size() << " terrain transitions" << std::endl;
#endif

		for (sf::Vector2i const &p : this->markUpdateTerrainTransitions) {
			this->updateDirtTransition(p.x, p.y);
			this->updateGrassConcreteTransition(p.x, p.y);
			this->updateSandWaterTransition(p.x, p.y);
			this->updateGrassSandTransition(p.x, p.y);
			this->updateConcreteSandTransition(p.x, p.y);
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
				this->map->terrainsForTransitions.set(x, y, tiles["dirt"][0]);

				if (res < -0.3) {
					t = tiles["sand"][rand() % ALT_TILES];
					this->map->staticBuildable.set(x, y, t);
					this->map->terrainsForTransitions.set(x, y, tiles["sand"][0]);
				}
				if (res < -0.5) {
					t = tiles["water"][rand() % ALT_TILES];

					this->map->staticBuildable.set(x, y, t);
					this->map->staticPathfinding.set(x, y, t);
					this->map->terrainsForTransitions.set(x, y, tiles["water"][0]);
				}

				this->map->terrains.set(x, y, t);

				// add some random ressources
				if (res > 0.6 && res < 0.61) {
					float rnd = ((float) rand()) / (float) RAND_MAX;
					if (rnd > 0.5) {
						this->vault->factory.plantResource(this->vault->registry, "nature", x, y);
					} else {
						this->vault->factory.plantResource(this->vault->registry, "pollution", x, y);
					}
				}
			}
		}

		for (auto pair : this->vault->factory.decorGenerator) {
			for (int i = 0; i < (this->map->width * this->map->height) / pair.second; i++) {
				int rx = rand() % this->map->width;
				int ry = rand() % this->map->height;
				if (this->map->terrainsForTransitions.get(rx, ry) != tiles["water"][0]) {
					this->vault->factory.createDecor(this->vault->registry, pair.first, rx, ry);
				}
			}

		}

		// set decor layer
		auto decorView = this->vault->registry.persistent<Tile, Decor>();

		for (EntityID entity : decorView) {
			Tile &tile = decorView.get<Tile>(entity);

			for (sf::Vector2i const &p : this->tileSurface(tile)) {
				this->map->decors.set(p.x, p.y, entity);
				this->map->staticBuildable.set(p.x, p.y, entity);
			}
		}


		this->updateAllTransitions();
	}

};