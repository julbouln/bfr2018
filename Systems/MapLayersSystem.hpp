#pragma once
#include "Helpers.hpp"
#include "GameSystem.hpp"
#include "TileMap.hpp"

class MapLayersSystem : public GameSystem {
	std::map<std::string, std::vector<EntityID>> tiles;

	std::vector<EntityID> dirtTransitions;
	std::vector<EntityID> waterTransitions;
	std::vector<EntityID> sandTransitions;
	std::vector<EntityID> concreteTransitions;

	std::map<int, int> terrainTransitionsMapping;
	std::vector<EntityID> fogTransitions;
	std::map<int, int> fogTransitionsMapping;

//	std::vector<EntityID> debugTransitions;

	// transitions calculation optimization
	// maintain a list of position to update instead of updating every transitions

	std::set<sf::Vector2i, CompareVector2i> markUpdateTerrainTransitions;
	std::set<sf::Vector2i, CompareVector2i> markUpdateFogTransitions;

	std::map<EntityID, EntityID> altTerrains;

	TileMap terrainsTileMap;
	TileMap fogTileMap;

public:
	void update(float dt) {
		this->updateLayer(dt);
		this->updateTransitions(dt);
		this->updateTileMap(dt);
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
						newEnt = Grass;
					} else if (obj.team == "neonaz") {
						newEnt = Concrete;
					}

					if (this->map->terrainsForTransitions.get(p.x, p.y) != newEnt) {
						for (sf::Vector2i const &sp : this->vectorSurfaceExtended(p, 1)) {
							this->markUpdateTerrainTransitions.insert(sp);
						}

						if (this->map->staticBuildable.get(p.x, p.y) == 0) {
							this->map->terrains[Terrain].set(p.x, p.y, newEnt);
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
					newEnt = Grass;
				} else {
					newEnt = Concrete;
				}

				if (this->map->terrainsForTransitions.get(p.x, p.y) != newEnt) {
					for (sf::Vector2i const &sp : this->vectorSurfaceExtended(p, 1)) {
						this->markUpdateTerrainTransitions.insert(sp);
					}

					if (this->map->staticBuildable.get(p.x, p.y) == 0) {
						this->map->terrains[Terrain].set(p.x, p.y, newEnt);
						this->map->terrainsForTransitions.set(p.x, p.y, newEnt);
					}
				}
			}
		}

	}

	void updateTileMap(float dt) {
		for (auto &layer : terrainsTileMap.layers) {
			layer.clear();
		}

		for (int y = 0; y < this->map->height; y++) {
			for (int x = 0; x < this->map->width; x++) {
				terrainsTileMap.layers[Terrain].addPosition(this->map->terrains[Terrain].get(x, y), sf::Vector2i(x, y));

				if (this->map->terrains[GrassConcrete].get(x, y))
					terrainsTileMap.layers[GrassConcrete].addPosition(this->map->terrains[GrassConcrete].get(x, y), sf::Vector2i(x, y));
				if (this->map->terrains[SandWater].get(x, y))
					terrainsTileMap.layers[SandWater].addPosition(this->map->terrains[SandWater].get(x, y), sf::Vector2i(x, y));
				if (this->map->terrains[GrassSand].get(x, y))
					terrainsTileMap.layers[GrassSand].addPosition(this->map->terrains[GrassSand].get(x, y), sf::Vector2i(x, y));
				if (this->map->terrains[ConcreteSand].get(x, y))
					terrainsTileMap.layers[ConcreteSand].addPosition(this->map->terrains[ConcreteSand].get(x, y), sf::Vector2i(x, y));
				if (this->map->terrains[AnyDirt].get(x, y))
					terrainsTileMap.layers[AnyDirt].addPosition(this->map->terrains[AnyDirt].get(x, y), sf::Vector2i(x, y));
			}
		}

		for (auto &layer : fogTileMap.layers) {
			layer.clear();
		}

		for (int y = 0; y < this->map->height; y++) {
			for (int x = 0; x < this->map->width; x++) {
				if (this->map->fogHidden.get(x, y) != 15)
					fogTileMap.layers[0].addPosition(this->map->fogHidden.get(x, y), sf::Vector2i(x, y));
				if (this->map->fogHiddenTransitions.get(x, y) != 15)
					fogTileMap.layers[1].addPosition(this->map->fogHiddenTransitions.get(x, y), sf::Vector2i(x, y));
				if (this->map->fogUnvisited.get(x, y) != 15)
					fogTileMap.layers[2].addPosition(this->map->fogUnvisited.get(x, y), sf::Vector2i(x, y));
				if (this->map->fogUnvisitedTransitions.get(x, y) != 15)
					fogTileMap.layers[3].addPosition(this->map->fogUnvisitedTransitions.get(x, y), sf::Vector2i(x, y));
			}
		}
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
			for (FogState &state : player.fog.grid) {
				if (state == FogState::InSight)
					state = FogState::Hidden;
			}
		}

		auto view = this->vault->registry.persistent<Tile, GameObject>();
		for (EntityID entity : view) {
			Tile &tile = view.get<Tile>(entity);
			GameObject &obj = view.get<GameObject>(entity);
			Player &player = this->vault->registry.get<Player>(obj.player);

			if (obj.mapped) {
				sf::IntRect surfRect = this->tileSurfaceExtendedRect(tile, obj.view);

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
				int newEnt = 0;

				if (st == FogState::Unvisited) {
//					newEnt = fogTransitions[0];
					newEnt = NotVisible;
				} else {
					newEnt = Visible;
				}

				if (this->map->fogUnvisited.get(x, y) != newEnt)
				{
					markUpdate = true;
				}

				this->map->fogUnvisited.set(x, y, newEnt);

				if (st == FogState::Hidden) {
					newEnt = NotVisible;
				} else {
					newEnt = Visible;
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

	void drawTerrainTileMap(sf::RenderWindow &window, float dt) {
		for (auto &layer : terrainsTileMap.layers) {
			window.draw(layer);
		}
	}

	void drawFogTileMap(sf::RenderWindow &window, float dt) {
		for (auto &layer : fogTileMap.layers) {
			window.draw(layer);
		}
	}

	void initTileMaps() {
		terrainsTileMap.resize(6);

		for (int i = 0; i < 15; i++) {
			terrainsTileMap.layers[Terrain].addTileRect(sf::IntRect((i / 3) * 32, (i % 3) * 32, 32, 32));
		}
		terrainsTileMap.layers[Terrain].init(&this->vault->factory.getTex("terrains"));

		for (int i = 0; i < 32; i++) {
			terrainsTileMap.layers[GrassConcrete].addTileRect(sf::IntRect(128, i * 32, 32, 32));
		}
		terrainsTileMap.layers[GrassConcrete].init(&this->vault->factory.getTex("terrains_transitions"));

		for (int i = 0; i < 32; i++) {
			terrainsTileMap.layers[SandWater].addTileRect(sf::IntRect(32, i * 32, 32, 32));
		}
		terrainsTileMap.layers[SandWater].init(&this->vault->factory.getTex("terrains_transitions"));

		for (int i = 0; i < 32; i++) {
			terrainsTileMap.layers[GrassSand].addTileRect(sf::IntRect(0, i * 32, 32, 32));
		}
		terrainsTileMap.layers[GrassSand].init(&this->vault->factory.getTex("terrains_transitions"));

		for (int i = 0; i < 32; i++) {
			terrainsTileMap.layers[ConcreteSand].addTileRect(sf::IntRect(0, i * 32, 32, 32));
		}
		terrainsTileMap.layers[ConcreteSand].init(&this->vault->factory.getTex("terrains_transitions"));

		for (int i = 0; i < 32; i++) {
			terrainsTileMap.layers[AnyDirt].addTileRect(sf::IntRect(96, i * 32, 32, 32));
		}
		terrainsTileMap.layers[AnyDirt].init(&this->vault->factory.getTex("terrains_transitions"));

		// FOG
		fogTileMap.resize(4);

		for (int i = 0; i < 13; i++) {
			fogTileMap.layers[0].addTileRect(sf::IntRect(0, i * 32, 32, 32));
			fogTileMap.layers[1].addTileRect(sf::IntRect(0, i * 32, 32, 32));
			fogTileMap.layers[2].addTileRect(sf::IntRect(0, i * 32, 32, 32));
			fogTileMap.layers[3].addTileRect(sf::IntRect(0, i * 32, 32, 32));
		}
		fogTileMap.layers[0].init(&this->vault->factory.getTex("fog_transition"), sf::Color(0x00, 0x00, 0x00, 0x7f));
		fogTileMap.layers[1].init(&this->vault->factory.getTex("fog_transition"), sf::Color(0x00, 0x00, 0x00, 0x7f));
		fogTileMap.layers[2].init(&this->vault->factory.getTex("fog_transition"), sf::Color(0x00, 0x00, 0x00, 0xff));
		fogTileMap.layers[3].init(&this->vault->factory.getTex("fog_transition"), sf::Color(0x00, 0x00, 0x00, 0xff));

	}

	void initTransitions() {
		for (int i = 0; i < 32; i++) {
			sandTransitions.push_back(i);
			waterTransitions.push_back(i);
			dirtTransitions.push_back(i);
			concreteTransitions.push_back(i);
		}

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
			fogTransitions.push_back(i);
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

//		for (int i = 0; i < 256; i++) {
//			debugTransitions.push_back(this->vault->factory.createTerrain(this->vault->registry, "debug_transition", i));
//		}

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

		tile.sprite.setTextureRect(sf::IntRect(0, ((this->vault->factory.getTex(name).getSize().y / tile.psize.y) - 1)*tile.psize.y, tile.psize.x, tile.psize.y)); // texture need to be updated

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

		std::vector<EntityID> tvec;

		for (int i = 0; i < 2; i++) {
			int ruinHeight = this->vault->factory.getTex("ruin").getSize().y / 2;
			Tile ruinTile;
			ruinTile.pos = sf::Vector2i(0, 0);
			ruinTile.ppos = sf::Vector2f(ruinTile.pos) * (float)32.0;
			ruinTile.shader = false;
			ruinTile.psize = sf::Vector2f(this->vault->factory.getTex("ruin").getSize().x, ruinHeight);
			ruinTile.sprite.setTexture(this->vault->factory.getTex("ruin"));
			ruinTile.centerRect = this->vault->factory.getCenterRect("ruin");
			ruinTile.sprite.setTextureRect(sf::IntRect(0, i*ruinHeight, ruinTile.psize.x, ruinTile.psize.y)); // texture need to be updated

			ruinTile.state = "idle";

			tvec.push_back(this->vault->registry.create());
			this->vault->registry.assign<Tile>(tvec.back(), ruinTile);
		}
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
					int trans = mapping[bitmask & 0xf];
					outLayer.set(x, y, trans);
				} else {
//					outLayer.set(x, y, debugTransitions[bitmask & 0xf]);
				}
			} else {
				if (mapping.count(bitmask) > 0) {
					int trans = mapping[bitmask];
					outLayer.set(x, y, trans);
				} else {
//					outLayer.set(x, y, debugTransitions[bitmask]);
				}
			}
		}
		return bitmask;
	}

	void updateGrassConcreteTransition(int x, int y) {
		int bitmask = this->pairTransitionBitmask(this->map->terrainsForTransitions, Grass, Concrete, x, y);
		bitmask = this->updateTransition(bitmask, this->map->terrains[GrassConcrete], Concrete, concreteTransitions, terrainTransitionsMapping, x, y);
		if (!bitmask) {
			this->map->terrains[GrassConcrete].set(x, y, 0);
		}
	}

	void updateSandWaterTransition(int x, int y) {
		int bitmask = this->pairTransitionBitmask(this->map->terrainsForTransitions, Sand, Water, x, y);
		bitmask = this->updateTransition(bitmask, this->map->terrains[SandWater], Water, waterTransitions, terrainTransitionsMapping, x, y);
		if (!bitmask) {
			this->map->terrains[SandWater].set(x, y, 0);
		}
	}

	void updateGrassSandTransition(int x, int y) {
		int bitmask = this->pairTransitionBitmask(this->map->terrainsForTransitions, Grass, Sand, x, y);
		bitmask = this->updateTransition(bitmask, this->map->terrains[GrassSand], Sand, sandTransitions, terrainTransitionsMapping, x, y);
		if (!bitmask) {
			this->map->terrains[GrassSand].set(x, y, 0);
		}
	}

	void updateConcreteSandTransition(int x, int y) {
		int bitmask = this->pairTransitionBitmask(this->map->terrainsForTransitions, Concrete, Sand, x, y);
		bitmask = this->updateTransition(bitmask, this->map->terrains[ConcreteSand], Sand, sandTransitions, terrainTransitionsMapping, x, y);
		if (!bitmask) {
			this->map->terrains[ConcreteSand].set(x, y, 0);
		}
	}

	void updateDirtTransition(int x, int y) {
		int bitmask = this->voidTransitionBitmask(this->map->terrainsForTransitions, Dirt, x, y);
		bitmask = this->updateTransition(bitmask, this->map->terrains[AnyDirt], Dirt, dirtTransitions, terrainTransitionsMapping, x, y);
		if (!bitmask) {
			this->map->terrains[AnyDirt].set(x, y, 0);
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

	void updateTransitions(float dt) {
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
		int bitmask = this->voidTransitionBitmask(this->map->fogUnvisited, NotVisible, x, y);
		bitmask = this->updateTransition(bitmask, this->map->fogUnvisitedTransitions, NotVisible, fogTransitions, fogTransitionsMapping, x, y);
		if (!bitmask) {
			this->map->fogUnvisitedTransitions.set(x, y, Visible);
		}
	}

	void updateFogHiddenTransition(int x, int y) {
		int bitmask = this->voidTransitionBitmask(this->map->fogHidden, NotVisible, x, y);
		bitmask = this->updateTransition(bitmask, this->map->fogHiddenTransitions, NotVisible, fogTransitions, fogTransitionsMapping, x, y);
		if (!bitmask) {
			this->map->fogHiddenTransitions.set(x, y, Visible);
		}
	}

};