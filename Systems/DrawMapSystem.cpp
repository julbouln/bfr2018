#include "DrawMapSystem.hpp"

DrawMapSystem::DrawMapSystem() {
}

void DrawMapSystem::init() {
	this->initTileMaps();
	this->updateAllTileMaps();
}

void DrawMapSystem::draw(sf::RenderWindow &window, sf::IntRect clip, float dt) {
	this->drawTerrainTileMap(window, dt);
	this->drawTileLayers(window, clip, dt);
	this->drawObjLayer(window, clip, dt);
	this->drawDebug(window, clip, dt);
}

void DrawMapSystem::drawSpriteWithShader(sf::RenderTarget & target, sf::Sprite & sprite, std::string shaderName, ShaderOptions & options) {
	sf::Shader *shader = this->vault->factory.shrManager.getRef(shaderName);
	applyShaderOptions(shader, options);
	target.draw(sprite, shader);
}

void DrawMapSystem::drawEntityLayer(sf::RenderTarget & target, Layer<EntityID> & layer, sf::IntRect clip, float dt, sf::Color colorVariant) {
	// UGLY: add one line of tile to draw to avoid half tile crop
	if (clip.top + clip.height + 1 < this->map->height)
		clip.height++;

	if (clip.left + clip.width + 1 < this->map->width)
		clip.width++;

	for (int y = clip.top; y < clip.top + clip.height; ++y) {
		for (int x = clip.left; x < clip.left + clip.width; ++x) {
			EntityID ent = layer.get(x, y);
			if (ent) {
				Tile &tile = this->vault->registry.get<Tile>(ent);

				sf::Vector2f pos;
				pos.x = x * 32 - (tile.centerRect.left + tile.centerRect.width / 2) + 16;
				pos.y = y * 32 - (tile.centerRect.top + tile.centerRect.height / 2) + 16;

				tile.sprite.setPosition(pos);
				tile.sprite.setColor(colorVariant);

				/* Draw the tile */
#ifdef SHADER_ENABLE
				if (tile.shader)
					this->drawSpriteWithShader(target, tile.sprite, tile.shaderName, tile.shaderOptions);
				else
					target.draw(tile.sprite);
#else
				target.draw(tile.sprite);
#endif
			}
		}

	}
}

void DrawMapSystem::drawTileLayers(sf::RenderTarget & target, sf::IntRect clip, float dt) {
	this->drawEntityLayer(target, this->map->corpses, clip, dt);
}

// reduce object list to visible entities
void DrawMapSystem::updateObjsDrawList(sf::RenderWindow & window, sf::IntRect clip, float dt) {
	this->entitiesDrawList.clear();

	// resources draw list
	auto resView = this->vault->registry.persistent<Tile, Resource>();

	for (EntityID entity : resView) {
		Tile &tile = resView.get<Tile>(entity);

		for (sf::Vector2i const &p : this->tileSurface(tile)) {
			if (this->map->fogHidden.get(p.x, p.y) == Visible && this->map->fogUnvisited.get(p.x, p.y) == Visible) {
				if (this->clipped(clip, p))
					this->entitiesDrawList.push_back(entity);
			}
		}
	}

	// game objects draw list
	auto view = this->vault->registry.persistent<Tile, GameObject>();

	for (EntityID entity : view) {
		Tile &tile = view.get<Tile>(entity);
		GameObject &obj = view.get<GameObject>(entity);

		for (sf::Vector2i const &p : this->tileSurface(tile)) {
			if (!obj.mapped || (this->map->fogHidden.get(p.x, p.y) == Visible && this->map->fogUnvisited.get(p.x, p.y) == Visible)) {
				if (this->clipped(clip, p))
					this->entitiesDrawList.push_back(entity);
			}
		}
	}

	// decor draw list
	auto decorView = this->vault->registry.persistent<Tile, Decor>();

	for (EntityID entity : decorView) {
		Tile &tile = decorView.get<Tile>(entity);
		Decor &decor = decorView.get<Decor>(entity);

		for (sf::Vector2i const &p : this->tileSurface(tile)) {
			if ((this->map->fogHidden.get(p.x, p.y) == Visible && this->map->fogUnvisited.get(p.x, p.y) == Visible)) {
				if (this->clipped(clip, p))
					this->entitiesDrawList.push_back(entity);
			}
		}
	}

	// sort by EntityID and uniq
	std::sort(this->entitiesDrawList.begin(), this->entitiesDrawList.end());
	auto last = std::unique(this->entitiesDrawList.begin(), this->entitiesDrawList.end());
	this->entitiesDrawList.erase(last, this->entitiesDrawList.end());

	// remove if invalid

	this->entitiesDrawList.erase(std::remove_if(
	                                 this->entitiesDrawList.begin(), this->entitiesDrawList.end(),
	[this](const EntityID & ent) {
		return (ent == 0 || !vault->registry.valid(ent) || !vault->registry.has<Tile>(ent));
	}), this->entitiesDrawList.end());

	// sort by position
	std::sort( this->entitiesDrawList.begin( ), this->entitiesDrawList.end(), [this ]( const auto & lhs, const auto & rhs )
	{
		Tile &lht = vault->registry.get<Tile>(lhs);
		Tile &rht = vault->registry.get<Tile>(rhs);
		sf::Vector2f lp = this->tileDrawPosition(lht);
		sf::Vector2f rp = this->tileDrawPosition(rht);
		if (lht.z < rht.z) {
			return true;
		} else {
			if (lht.z == rht.z) {
				int ly = lp.y + lht.centerRect.top + lht.centerRect.height / 2;
				int ry = rp.y + rht.centerRect.top + rht.centerRect.height / 2;
				if (ly < ry) {
					return true;
				} else {
					if (ly == ry) {
						int lx = lp.x + lht.centerRect.left + lht.centerRect.width / 2;
						int rx = rp.x + rht.centerRect.left + rht.centerRect.width / 2;
						if (lx < rx) {
							return true;
						} else {
							if (lx == rx)
								return (lht.psize.y < rht.psize.y);
						}
					}
				}
			}
		}
		return false;
	});
}


void DrawMapSystem::drawObjLayer(sf::RenderWindow & window, sf::IntRect clip, float dt) {
	this->updateObjsDrawList(window, clip, dt);

	for (EntityID ent : this->entitiesDrawList) {
		if (this->vault->registry.valid(ent)) { // DIRTY ?
			Tile &tile = this->vault->registry.get<Tile>(ent);

			// unit shadow
			if (this->vault->registry.has<Unit>(ent)) {
				sf::Sprite shadow;
				shadow.setTexture(this->vault->factory.getTex("shadow"));
				sf::Vector2f spos;

				spos.x = tile.ppos.x - 16;
				spos.y = tile.ppos.y - 16 + 13;

				shadow.setPosition(spos);
				window.draw(shadow);
			}

			sf::Vector2f pos = this->tileDrawPosition(tile);

			tile.sprite.setPosition(pos);

			if (this->vault->registry.has<GameObject>(ent)) {
				GameObject &obj = this->vault->registry.get<GameObject>(ent);
				Player &player = this->vault->registry.get<Player>(obj.player);

#ifdef SHADER_ENABLE
				if (tile.shader)
					this->drawSpriteWithShader(window, tile.sprite, tile.shaderName, tile.shaderOptions);
				else
					window.draw(tile.sprite);
#else
				window.draw(tile.sprite);
#endif
			} else {
				window.draw(tile.sprite);
			}

			// life bar
			if (this->vault->registry.has<GameObject>(ent)) {
				GameObject &obj = this->vault->registry.get<GameObject>(ent);
				if (obj.life > 0) {
					sf::Vector2f lpos;
					lpos.x = tile.ppos.x - 16.0f;
					lpos.y = tile.ppos.y - (tile.centerRect.top + tile.centerRect.height / 2) + tile.offset.y * 32 - 16.0f;

					sf::RectangleShape lifeBarFrame;
					lifeBarFrame.setSize(sf::Vector2f(32, 8));
					lifeBarFrame.setFillColor(sf::Color(0x00, 0x00, 0x00, 0x00));
					lifeBarFrame.setOutlineColor(sf::Color(0x00, 0x00, 0x00, 0xff));
					lifeBarFrame.setOutlineThickness(1);
					lifeBarFrame.setPosition(lpos);

					window.draw(lifeBarFrame);

					sf::RectangleShape lifeBar;

					float lifePer = (obj.life / obj.maxLife);
					sf::Color lifeCol = sf::Color(0x00, 0xff, 0x00, 0xff);
					if (lifePer < 0.75)
						lifeCol = sf::Color(0xff, 0xff, 0x00, 0xff);
					if (lifePer < 0.50)
						lifeCol = sf::Color(0xff, 0xa5, 0x00, 0xff);
					if (lifePer < 0.25)
						lifeCol = sf::Color(0xff, 0x00, 0x00, 0xff);


					lifeBar.setSize(sf::Vector2f(32 * lifePer, 8));
					lifeBar.setFillColor(lifeCol);
					lifeBar.setOutlineColor(sf::Color(0x00, 0x00, 0x00, 0x00));
					lifeBar.setOutlineThickness(1);
					lifeBar.setPosition(lpos);

					window.draw(lifeBar);
				}
			}
		}
	}
}

void DrawMapSystem::initTileMaps() {
	int w = this->map->width;
	int h = this->map->height;
	
	terrainsTileMap.resize(6, w, h);

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
	fogTileMap.resize(4, w, h);

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

void DrawMapSystem::updateAllTerrainTileMap(float dt) {
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

}
void DrawMapSystem::updateAllFogTileMap(float dt) {
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

void DrawMapSystem::updateAllTileMaps() {
	this->updateAllTerrainTileMap(0);
	this->updateAllFogTileMap(0);
}

void DrawMapSystem::drawTerrainTileMap(sf::RenderWindow &window, float dt) {
	for (auto &layer : terrainsTileMap.layers) {
		window.draw(layer);
	}
}

void DrawMapSystem::drawFogTileMap(sf::RenderWindow &window, float dt) {
	for (auto &layer : fogTileMap.layers) {
		window.draw(layer);
	}
}

void DrawMapSystem::update(float dt) {
	// only update updated terrain tiles
	for (sf::Vector2i const &p : this->map->markUpdateTerrainTransitions) {
		terrainsTileMap.layers[Terrain].setPosition(this->map->terrains[Terrain].get(p.x, p.y), p);
	}

	if (this->map->markUpdateTerrainTransitions.size() > 0) {
		terrainsTileMap.layers[GrassConcrete].clear();
		terrainsTileMap.layers[SandWater].clear();
		terrainsTileMap.layers[GrassSand].clear();
		terrainsTileMap.layers[ConcreteSand].clear();
		terrainsTileMap.layers[AnyDirt].clear();

		for (int y = 0; y < this->map->height; y++) {
			for (int x = 0; x < this->map->width; x++) {

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
	}
//		this->updateAllTerrainTileMap(dt);

	if (this->map->markUpdateFogTransitions.size() > 0)
		this->updateAllFogTileMap(dt);
}

// draw debug grid
void DrawMapSystem::drawDebug(sf::RenderWindow & window, sf::IntRect clip, float dt) {
	GameController &controller = this->vault->registry.get<GameController>();
	if (controller.showDebugWindow) {
		for (int y = clip.top; y < clip.top + clip.height; ++y)
		{
			for (int x = clip.left; x < clip.left + clip.width; ++x)
			{
				EntityID objEnt = this->map->objs.get(x, y);
				// objects
				if (objEnt) {
					sf::RectangleShape rectangle;

					sf::Vector2f pos;
					pos.x = x * 32;
					pos.y = y * 32;

					rectangle.setSize(sf::Vector2f(32, 32));
					rectangle.setFillColor(sf::Color(0x00, 0x00, 0x00, 0x00));
					rectangle.setOutlineColor(sf::Color(0xff, 0xff, 0xff, 0x7f));
					rectangle.setOutlineThickness(1);
					rectangle.setPosition(pos);

					window.draw(rectangle);
				}

				// resources
				if (this->map->resources.get(x, y)) {
					sf::RectangleShape rectangle;

					sf::Vector2f pos;
					pos.x = x * 32;
					pos.y = y * 32;

					rectangle.setSize(sf::Vector2f(32, 32));
					rectangle.setFillColor(sf::Color(0x00, 0x00, 0x00, 0x00));
					rectangle.setOutlineColor(sf::Color(0x00, 0xff, 0x00, 0x7f));
					rectangle.setOutlineThickness(1);
					rectangle.setPosition(pos);

					window.draw(rectangle);
				}
			}
		}

		if (controller.selectedDebugObj) {
			if (this->vault->registry.valid(controller.selectedDebugObj)) {
				Tile &tile = this->vault->registry.get<Tile>(controller.selectedDebugObj);

				// draw surface case
				for (sf::Vector2i const &p : this->tileSurface(tile)) {
					sf::RectangleShape srect;

					sf::Vector2f pos;
					pos.x = p.x * 32;
					pos.y = p.y * 32;

					srect.setSize(sf::Vector2f(32, 32));
					srect.setFillColor(sf::Color(0x00, 0x00, 0x00, 0x00));
					srect.setOutlineColor(sf::Color(0x00, 0xFF, 0xFF, 0xFF));
					srect.setOutlineThickness(2);
					srect.setPosition(pos);

					window.draw(srect);
				}

				// view range

				if (this->vault->registry.has<GameObject>(controller.selectedDebugObj)) {
					GameObject &obj = this->vault->registry.get<GameObject>(controller.selectedDebugObj);
					for (sf::Vector2i const &p : this->tileSurfaceExtended(tile, obj.view)) {
						sf::RectangleShape srect;

						sf::Vector2f pos;
						pos.x = p.x * 32;
						pos.y = p.y * 32;

						srect.setSize(sf::Vector2f(32, 32));
						srect.setFillColor(sf::Color(0x00, 0x00, 0x00, 0x00));
						srect.setOutlineColor(sf::Color(0xff, 0xff, 0x00, 0x7f));
						srect.setOutlineThickness(2);
						srect.setPosition(pos);

						window.draw(srect);
					}
				}

				// attack range
				if (this->vault->registry.has<Unit>(controller.selectedDebugObj)) {
					Unit &unit = this->vault->registry.get<Unit>(controller.selectedDebugObj);
					int dist = 1;
					int maxDist = 1;
					if (unit.attack2.distance) {
						dist = unit.attack2.distance;
						maxDist = unit.attack2.maxDistance;
					}

					for (sf::Vector2i const &p : this->tileAround(tile, dist, maxDist)) {
						sf::RectangleShape srect;

						sf::Vector2f pos = sf::Vector2f(p * 32);

						srect.setSize(sf::Vector2f(32, 32));
						srect.setFillColor(sf::Color(0x00, 0x00, 0x00, 0x00));
						srect.setOutlineColor(sf::Color(0xff, 0x99, 0x33, 0xff));
						srect.setOutlineThickness(2);
						srect.setPosition(pos);

						window.draw(srect);
					}

					sf::Vector2f sppos = tile.ppos - dist * 32.0f;

					sf::CircleShape scircle;
					scircle.setRadius(dist * 32.0f);
					scircle.setFillColor(sf::Color(0x00, 0x00, 0x00, 0x00));
					scircle.setOutlineColor(sf::Color(0xff, 0x99, 0x33, 0xff));
					scircle.setOutlineThickness(2);
					scircle.setPosition(sppos);

					window.draw(scircle);

					sf::Vector2f eppos = tile.ppos - maxDist * 32.0f;

					sf::CircleShape ecircle;
					ecircle.setRadius(maxDist * 32.0f);
					ecircle.setFillColor(sf::Color(0x00, 0x00, 0x00, 0x00));
					ecircle.setOutlineColor(sf::Color(0xff, 0x99, 0x33, 0xff));
					ecircle.setOutlineThickness(2);
					ecircle.setPosition(eppos);

					window.draw(ecircle);

				}

				// around building
				if (this->vault->registry.has<Building>(controller.selectedDebugObj)) {
					Building &building = this->vault->registry.get<Building>(controller.selectedDebugObj);
					int dist = 1;
					int maxDist = 2;
					for (sf::Vector2i const &p : this->tileAround(tile, dist, maxDist)) {
						sf::RectangleShape srect;

						sf::Vector2f pos;
						pos.x = p.x * 32;
						pos.y = p.y * 32;

						srect.setSize(sf::Vector2f(32, 32));
						srect.setFillColor(sf::Color(0x00, 0x00, 0x00, 0x00));
						srect.setOutlineColor(sf::Color(0xff, 0x99, 0x33, 0xff));
						srect.setOutlineThickness(2);
						srect.setPosition(pos);

						window.draw(srect);
					}
				}

				// draw tile case
				sf::RectangleShape trect;
				sf::Vector2f pos;
				pos.x = tile.pos.x * 32;
				pos.y = tile.pos.y * 32;
				trect.setSize(sf::Vector2f(32, 32));
				trect.setFillColor(sf::Color(0x00, 0x00, 0x00, 0x00));
				trect.setOutlineColor(sf::Color::Blue);
				trect.setOutlineThickness(2);
				trect.setPosition(pos);
				window.draw(trect);

				// draw pixel rect
				sf::RectangleShape drect;
				sf::Vector2f dpos = this->tileDrawPosition(tile);
				drect.setSize(tile.psize);
				drect.setFillColor(sf::Color(0x00, 0x00, 0x00, 0x00));
				drect.setOutlineColor(sf::Color(0xff, 0xb6, 0xc1, 0xff));
				drect.setOutlineThickness(2);
				drect.setPosition(dpos);
				window.draw(drect);

				// draw center rect
				sf::RectangleShape crect;
				sf::Vector2f cpos;
				cpos.x = dpos.x + tile.centerRect.left;
				cpos.y = dpos.y + tile.centerRect.top;
				crect.setSize(sf::Vector2f(tile.centerRect.width, tile.centerRect.height));
				crect.setFillColor(sf::Color(0x00, 0x00, 0x00, 0x00));
				crect.setOutlineColor(sf::Color(255, 36, 196));
				crect.setOutlineThickness(2);
				crect.setPosition(cpos);
				window.draw(crect);

				// draw FlowFields
				if (this->vault->registry.has<Unit>(controller.selectedDebugObj)) {
					Tile &tile = this->vault->registry.get<Tile>(controller.selectedDebugObj);
					Unit &unit = this->vault->registry.get<Unit>(controller.selectedDebugObj);
					if (tile.pos != unit.destpos) {
						sf::Vector2i fOffset = unit.flowFieldPath.offset(tile.pos);
						FlowField *field = unit.flowFieldPath.getCurrentFlowField();
						if (field) {
							sf::Vector2i fsize = field->getSize();
							for (int dx = 0; dx < fsize.x; dx++) {
								for (int dy = 0; dy < fsize.y; dy++) {
									int dir = field->get(dx, dy);
									if (dir < 8) {
										sf::Sprite dirSprite;
										dirSprite.setColor(sf::Color(0x00, 0x00, 0xff, 0x7f));
										dirSprite.setTexture(this->vault->factory.getTex("arrow"));
										dirSprite.setOrigin(sf::Vector2f(16, 16));
										switch (dir) {
										case 0:
											dirSprite.setRotation(90);
											break;
										case 1:
											dirSprite.setRotation(135);
											break;
										case 2:
											dirSprite.setRotation(180);
											break;
										case 3:
											dirSprite.setRotation(225);
											break;
										case 4:
											dirSprite.setRotation(270);
											break;
										case 5:
											dirSprite.setRotation(315);
											break;
										case 6:
											dirSprite.setRotation(0);
											break;
										case 7:
											dirSprite.setRotation(45);
											break;
										}

										sf::Vector2f dppos((fOffset.x + dx) * 32 + 16, (fOffset.y + dy) * 32 + 16);
										dirSprite.setPosition(dppos);
										window.draw(dirSprite);
									}
								}
							}


							for (sf::Vector2i p : unit.flowFieldPath.inRangePathPoints(tile.pos)) {
								sf::RectangleShape rectangle;

								sf::Vector2f pos(p.x * 32, p.y * 32);

								rectangle.setSize(sf::Vector2f(32, 32));
								rectangle.setFillColor(sf::Color(0x00, 0x00, 0x00, 0x00));
								rectangle.setOutlineColor(sf::Color(0x00, 0x00, 0xff, 0x7f));
								rectangle.setOutlineThickness(2);
								rectangle.setPosition(pos);

								window.draw(rectangle);
							}

							sf::RectangleShape rectangle;

							sf::Vector2f pos(unit.flowFieldPath.ffDest.x * 32, unit.flowFieldPath.ffDest.y * 32);

							rectangle.setSize(sf::Vector2f(32, 32));
							rectangle.setFillColor(sf::Color(0x00, 0x00, 0xff, 0x7f));
							rectangle.setPosition(pos);

							window.draw(rectangle);

						}
					}
				}
			} else {
				controller.selectedDebugObj = 0;
			}
		}
	}

}
