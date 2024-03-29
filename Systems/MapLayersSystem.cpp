#include "MapLayersSystem.hpp"

void MapLayersSystem::init() {
	this->initTransitions();
	this->updateAllTransitions();
}

void MapLayersSystem::update(float dt) {
	this->updateLayer(dt);
//		this->updateTileMap(dt);
	this->updateObjsLayer(dt);
	this->updatePlayersFog(dt);
	this->updateTransitions(dt);
}

void MapLayersSystem::updateFog(float dt) {
	GameController &controller = this->vault->registry.get<GameController>();

	this->updateSpectatorFog(controller.currentPlayer, dt);
	this->updatePlayerFogLayer(controller.currentPlayer, dt);
}

void MapLayersSystem::updateLayer(float dt) {
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
						this->map->markUpdateTerrainTransitions.insert(sp);
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
					this->map->markUpdateTerrainTransitions.insert(sp);
				}

				if (this->map->staticBuildable.get(p.x, p.y) == 0) {
					this->map->terrains[Terrain].set(p.x, p.y, newEnt);
					this->map->terrainsForTransitions.set(p.x, p.y, newEnt);
				}
			}
		}
	}

}

void MapLayersSystem::updateObjsLayer(float dt) {
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

void MapLayersSystem::updatePlayersFog(float dt) {
	auto playerView = this->vault->registry.view<Player>();

	for (EntityID entity : playerView) {
		Player &player = playerView.get(entity);
//			for (FogState &state : player.fog.grid) {
		for (int i = 0; i < player.fog.size(); ++i) {
			FogState &state = player.fog.grid[i];
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
//				sf::IntRect surfRect = this->tileSurfaceExtendedRect(tile, obj.view);

			for (sf::Vector2i const &p : this->tileSurfaceExtended(tile, obj.view)) {
				player.fog.set(p.x, p.y, FogState::InSight);
			}
		}

	}
}

// spectator FOG concat all other players fog
void MapLayersSystem::updateSpectatorFog(EntityID playerEnt, float dt) {
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

void MapLayersSystem::updatePlayerFogLayer(EntityID playerEnt, float dt) {
	Player &player = this->vault->registry.get<Player>(playerEnt);

	for (int y = 0; y < this->map->height; ++y) {
		for (int x = 0; x < this->map->width; ++x) {
			sf::Vector2i p = sf::Vector2i(x, y);
			FogState st = player.fog.get(x, y);
			bool markUpdate = false;
			int newEnt = 0;

			if (st == FogState::Unvisited) {
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
					this->map->markUpdateFogTransitions.insert(sp);
				}
			}
		}
	}

#ifdef TRANSITIONS_DEBUG
	std::cout << "Transitions: update " << this->map->markUpdateFogTransitions.size() << " FOG transitions" << std::endl;
#endif
}

// Terrains/Transitions
void MapLayersSystem::initTransitions() {
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

// https://gamedevelopment.tutsplus.com/tutorials/how-to-use-tile-bitmasking-to-auto-tile-your-level-layouts--cms-25673
int MapLayersSystem::transitionBitmask(Layer<int> & layer, EntityID ent, int x, int y) {
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

int MapLayersSystem::voidTransitionBitmask(Layer<int> & layer, EntityID ent, int x, int y) {
	int bitmask = 0;
	if (layer.get(x, y) != ent) {
		bitmask = this->transitionBitmask(layer, ent, x, y);
	}
	return bitmask;
}

int MapLayersSystem::pairTransitionBitmask(Layer<int> & layer, EntityID srcEnt, EntityID dstEnt, int x, int y) {
	int bitmask = 0;
	if (layer.get(x, y) == srcEnt) {
		bitmask = this->transitionBitmask(layer, dstEnt, x, y);
	}
	return bitmask;
}

int MapLayersSystem::updateTransition(int bitmask, Layer<int> & outLayer, EntityID ent, std::map<int, int> &mapping, int x, int y) {
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

void MapLayersSystem::updateGrassConcreteTransition(int x, int y) {
	int bitmask = this->pairTransitionBitmask(this->map->terrainsForTransitions, Grass, Concrete, x, y);
	bitmask = this->updateTransition(bitmask, this->map->terrains[GrassConcrete], Concrete, terrainTransitionsMapping, x, y);
	if (!bitmask) {
		this->map->terrains[GrassConcrete].set(x, y, 0);
	}
}

void MapLayersSystem::updateSandWaterTransition(int x, int y) {
	int bitmask = this->pairTransitionBitmask(this->map->terrainsForTransitions, Sand, Water, x, y);
	bitmask = this->updateTransition(bitmask, this->map->terrains[SandWater], Water, terrainTransitionsMapping, x, y);
	if (!bitmask) {
		this->map->terrains[SandWater].set(x, y, 0);
	}
}

void MapLayersSystem::updateGrassSandTransition(int x, int y) {
	int bitmask = this->pairTransitionBitmask(this->map->terrainsForTransitions, Grass, Sand, x, y);
	bitmask = this->updateTransition(bitmask, this->map->terrains[GrassSand], Sand, terrainTransitionsMapping, x, y);
	if (!bitmask) {
		this->map->terrains[GrassSand].set(x, y, 0);
	}
}

void MapLayersSystem::updateConcreteSandTransition(int x, int y) {
	int bitmask = this->pairTransitionBitmask(this->map->terrainsForTransitions, Concrete, Sand, x, y);
	bitmask = this->updateTransition(bitmask, this->map->terrains[ConcreteSand], Sand, terrainTransitionsMapping, x, y);
	if (!bitmask) {
		this->map->terrains[ConcreteSand].set(x, y, 0);
	}
}

void MapLayersSystem::updateDirtTransition(int x, int y) {
	int bitmask = this->voidTransitionBitmask(this->map->terrainsForTransitions, Dirt, x, y);
	bitmask = this->updateTransition(bitmask, this->map->terrains[AnyDirt], Dirt, terrainTransitionsMapping, x, y);
	if (!bitmask) {
		this->map->terrains[AnyDirt].set(x, y, 0);
	}
}

void MapLayersSystem::updateAllTransitions() {
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

void MapLayersSystem::updateTransitions(float dt) {
#ifdef TRANSITIONS_DEBUG
	std::cout << "Transitions: update " << this->markUpdateTerrainTransitions.size() << " terrain transitions" << std::endl;
#endif

	for (sf::Vector2i const &p : this->map->markUpdateTerrainTransitions) {
		this->updateDirtTransition(p.x, p.y);
		this->updateGrassConcreteTransition(p.x, p.y);
		this->updateSandWaterTransition(p.x, p.y);
		this->updateGrassSandTransition(p.x, p.y);
		this->updateConcreteSandTransition(p.x, p.y);
	}

	for (sf::Vector2i const &p : this->map->markUpdateFogTransitions) {
		this->updateFogHiddenTransition(p.x, p.y);
		this->updateFogUnvisitedTransition(p.x, p.y);
	}
}

// FOG transition

void MapLayersSystem::updateFogUnvisitedTransition(int x, int y) {
	int bitmask = this->voidTransitionBitmask(this->map->fogUnvisited, NotVisible, x, y);
	bitmask = this->updateTransition(bitmask, this->map->fogUnvisitedTransitions, NotVisible, fogTransitionsMapping, x, y);
	if (!bitmask) {
		this->map->fogUnvisitedTransitions.set(x, y, Visible);
	}
}

void MapLayersSystem::updateFogHiddenTransition(int x, int y) {
	int bitmask = this->voidTransitionBitmask(this->map->fogHidden, NotVisible, x, y);
	bitmask = this->updateTransition(bitmask, this->map->fogHiddenTransitions, NotVisible, fogTransitionsMapping, x, y);
	if (!bitmask) {
		this->map->fogHiddenTransitions.set(x, y, Visible);
	}
}
