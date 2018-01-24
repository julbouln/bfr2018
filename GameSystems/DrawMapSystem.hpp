#pragma once

#include "GameSystem.hpp"

class DrawMapSystem : public GameSystem {
public:
	sf::Shader colorSwap;
	sf::Shader pixelation;
	sf::Shader outline;

	std::vector<EntityID> entitiesDrawList;

	bool showDebugLayer;

	DrawMapSystem() {
		if (!colorSwap.loadFromFile("defs/new/shaders/color_swap.frag", sf::Shader::Fragment))
			std::cout << "ERROR: cannot load colorSwap shader" << std::endl;
		if (!pixelation.loadFromFile("defs/new/shaders/pixelation.frag", sf::Shader::Fragment))
			std::cout << "ERROR: cannot load pixelation shader" << std::endl;
		if (!outline.loadFromFile("defs/new/shaders/outline.frag", sf::Shader::Fragment))
			std::cout << "ERROR: cannot load pixelation shader" << std::endl;

		this->showDebugLayer = false;
	}

	void draw(sf::RenderWindow &window, sf::IntRect clip, float dt) {
		this->drawLayer(window, this->map->terrains, clip, dt);
		for (Layer &transitionLayer : this->map->transitions) {
			this->drawLayer(window, transitionLayer, clip, dt);
		}

		this->drawLayer(window, this->map->corpses, clip, dt);

		this->drawLayer(window, clip, dt);
		if (showDebugLayer)
			this->drawDebug(window, clip, dt);

		this->drawLayer(window, this->map->fogHidden, clip, dt, sf::Color(0x00, 0x00, 0x00, 0x7f));
		this->drawLayer(window, this->map->fogUnvisited, clip, dt, sf::Color(0x00, 0x00, 0x00));
		this->drawLayer(window, this->map->fogHiddenTransitions, clip, dt, sf::Color(0x00, 0x00, 0x00, 0x7f));
		this->drawLayer(window, this->map->fogUnvisitedTransitions, clip, dt, sf::Color(0x00, 0x00, 0x00));
	}

	void drawMinimap(sf::RenderTexture &target, EntityID playerEnt) {
		Player &player = this->vault->registry.get<Player>(playerEnt);

		target.clear(sf::Color::Black);

		for (int y = 0; y < this->map->height; ++y)
		{
			for (int x = 0; x < this->map->width; ++x)
			{
				FogState fogSt = player.fog.get(x, y);
				if (fogSt != FogState::Unvisited) {
					EntityID terrainEnt = this->map->terrains.get(x, y);
					if (terrainEnt) {
						sf::RectangleShape rectangle;
						sf::Vector2f pos(x, y);
						rectangle.setSize(sf::Vector2f(1, 1));
						if (fogSt != FogState::Hidden) {
							rectangle.setFillColor(sf::Color(0x63, 0x4d, 0x0a, 0xff));
						} else {
							rectangle.setFillColor(sf::Color(0x63, 0x4d, 0x0a, 0x7f));
						}
						rectangle.setPosition(pos);
						target.draw(rectangle);
					}

					if (fogSt != FogState::Hidden) {
						EntityID objEnt = this->map->objs.get(x, y);
						if (objEnt) {
							sf::RectangleShape rectangle;
							sf::Vector2f pos(x, y);
							rectangle.setSize(sf::Vector2f(1, 1));
							rectangle.setFillColor(sf::Color(0xff, 0xff, 0xff, 0xff));
							rectangle.setPosition(pos);
							target.draw(rectangle);
						}
					}
				}
			}
		}

		// frame rectangle
		sf::RectangleShape r;
		sf::Vector2f rPos(1, 1);
		r.setSize(sf::Vector2f(this->map->width - 2, this->map->height - 2));
		r.setFillColor(sf::Color(0x00, 0x00, 0x00, 0x00));
		r.setOutlineColor(sf::Color(0x66, 0x66, 0x66, 0xff));
		r.setOutlineThickness(1);
		r.setPosition(rPos);
		target.draw(r);

		target.display();
	}

	void drawMinimapClip(sf::RenderWindow &window, sf::IntRect clip) {
		// clip rectangle
		sf::RectangleShape clipR;
		sf::Vector2f clipPos(clip.left, clip.top);
		clipR.setSize(sf::Vector2f(clip.width, clip.height));
		clipR.setFillColor(sf::Color(0x00, 0x00, 0x00, 0x00));
		clipR.setOutlineColor(sf::Color(0xff, 0xff, 0xff, 0xff));
		clipR.setOutlineThickness(1);
		clipR.setPosition(clipPos);
		window.draw(clipR);
	}

	void drawLayer(sf::RenderWindow &window, Layer &layer, sf::IntRect clip, float dt, sf::Color colorVariant = sf::Color(0xff, 0xff, 0xff)) {
		for (int y = clip.top; y < clip.top + clip.height; ++y)
		{
			for (int x = clip.left; x < clip.left + clip.width; ++x)
			{
				EntityID ent = layer.get(x, y);
				if (ent) {
					Tile &tile = this->vault->registry.get<Tile>(ent);

					sf::Vector2f pos;
					pos.x = x * 32 - (tile.centerRect.left + tile.centerRect.width / 2) + 16;
					pos.y = y * 32 - (tile.centerRect.top + tile.centerRect.height / 2) + 16;
//					pos.x = x * 32;
//					pos.y = y * 32;

					tile.sprite.setPosition(pos);
					tile.sprite.setColor(colorVariant);

					/* Draw the tile */
					window.draw(tile.sprite);

//	pixelation.setParameter("texture", sf::Shader::CurrentTexture);
//	pixelation.setParameter("amount", 32.0);
//				window.draw(tile.sprite,&pixelation);
				}
			}

		}
	}

	// reduce object list to visible entities
	void updateObjsDrawList(sf::RenderWindow &window, sf::IntRect clip, float dt) {

		this->entitiesDrawList.clear();

		// resources draw list
		auto resView = this->vault->registry.persistent<Tile, Resource>();

		for (EntityID entity : resView) {
			Tile &tile = resView.get<Tile>(entity);

			for (sf::Vector2i p : this->tileSurface(tile)) {
				if (this->map->fogHidden.get(p.x, p.y) == 0 && this->map->fogUnvisited.get(p.x, p.y) == 0) {
					if (p.x >= clip.left && p.x <= clip.left + clip.width &&
					        p.y >= clip.top && p.y <= clip.top + clip.height)
						this->entitiesDrawList.push_back(entity);
				}
			}
		}


		// game objects draw list
		auto view = this->vault->registry.persistent<Tile, GameObject>();

		for (EntityID entity : view) {
			Tile &tile = view.get<Tile>(entity);

			for (sf::Vector2i p : this->tileSurface(tile)) {
				if (this->map->fogHidden.get(p.x, p.y) == 0 && this->map->fogUnvisited.get(p.x, p.y) == 0) {
					if (p.x >= clip.left && p.x <= clip.left + clip.width &&
					        p.y >= clip.top && p.y <= clip.top + clip.height)
						this->entitiesDrawList.push_back(entity);
				}
			}
		}

		// effects draw list
		auto fxView = this->vault->registry.persistent<Tile, MapEffect>();
		for (EntityID entity : fxView) {
			Tile &tile = fxView.get<Tile>(entity);
			MapEffect &effect = fxView.get<MapEffect>(entity);
			if (effect.show) {
				for (sf::Vector2i p : this->tileSurface(tile)) {
					if (this->map->fogHidden.get(p.x, p.y) == 0 && this->map->fogUnvisited.get(p.x, p.y) == 0) {
						if (p.x >= clip.left && p.x <= clip.left + clip.width &&
						        p.y >= clip.top && p.y <= clip.top + clip.height)
							this->entitiesDrawList.push_back(entity);
					}
				}
			}
		}

		// sort by EntityID and uniq
		std::sort(this->entitiesDrawList.begin(), this->entitiesDrawList.end());
		auto last = std::unique(this->entitiesDrawList.begin(), this->entitiesDrawList.end());
		this->entitiesDrawList.erase(last, this->entitiesDrawList.end());


		// remove if invalid
		/*
		this->entitiesDrawList.erase(std::remove_if(
		                                 this->entitiesDrawList.begin(), this->entitiesDrawList.end(),
		[this](const EntityID & ent) {
			return (ent==0 || !vault->registry.valid(ent) || !vault->registry.has<Tile>(ent));
		}), this->entitiesDrawList.end());
		*/

		// sort by position
		std::sort( this->entitiesDrawList.begin( ), this->entitiesDrawList.end(), [this ]( const auto & lhs, const auto & rhs )
		{
			Tile &lht = vault->registry.get<Tile>(lhs);
			Tile &rht = vault->registry.get<Tile>(rhs);
			if (lht.z < rht.z) {
				return true;
			} else {
				if (lht.z == rht.z) {
					int ly = lht.ppos.y + (lht.centerRect.top + lht.centerRect.height) / 2;
					int ry = rht.ppos.y + (rht.centerRect.top + rht.centerRect.height) / 2;
					if ( ly < ry) {
						return true;
					} else {
						if (ly == ry) {
							int lx = lht.ppos.x + (lht.centerRect.left + lht.centerRect.width) / 2;
							int rx = rht.ppos.x + (rht.centerRect.left + rht.centerRect.width) / 2;
							return (lx < rx);
						}
					}

				}
			}
			return false;
		});
	}

	void drawLayer(sf::RenderWindow &window, sf::IntRect clip, float dt) {
		this->updateObjsDrawList(window, clip, dt);

		for (EntityID ent : this->entitiesDrawList) {
			if (this->vault->registry.valid(ent)) { // DIRTY ?
				Tile &tile = this->vault->registry.get<Tile>(ent);

				// unit shadow
				if (this->vault->registry.has<Unit>(ent)) {
					sf::Sprite shadow;
					shadow.setTexture(this->vault->factory.getTex("shadow"));
					sf::Vector2f spos;

					spos.x = tile.ppos.x;
					spos.y = tile.ppos.y + 13;

					shadow.setPosition(spos);
					window.draw(shadow);
				}

				sf::Vector2f pos;

				pos.x = tile.ppos.x - (tile.centerRect.left + tile.centerRect.width / 2) + 16 + tile.offset.x * 32;
				pos.y = tile.ppos.y - (tile.centerRect.top + tile.centerRect.height / 2) + 16 + tile.offset.y * 32;

				tile.sprite.setPosition(pos);

				if (this->vault->registry.has<GameObject>(ent)) {
					GameObject &obj = this->vault->registry.get<GameObject>(ent);
					Player &player = this->vault->registry.get<Player>(obj.player);

					sf::Color col1 = sf::Color(3, 255, 205);
					sf::Color col2 = sf::Color(0, 235, 188);
					sf::Color replace1 = this->vault->factory.getPlayerColor(col1, player.colorIdx);
					sf::Color replace2 = this->vault->factory.getPlayerColor(col2, player.colorIdx);

					colorSwap.setParameter("texture", sf::Shader::CurrentTexture);
					colorSwap.setParameter("color1", col1);
					colorSwap.setParameter("replace1", replace1);
					colorSwap.setParameter("color2", col2);
					colorSwap.setParameter("replace2", replace2);

					window.draw(tile.sprite, &colorSwap);
				} else {
					window.draw(tile.sprite);

				}

				// life bar
				if (this->vault->registry.has<GameObject>(ent)) {
					GameObject &obj = this->vault->registry.get<GameObject>(ent);
					if (obj.life > 0) {
						sf::Vector2f lpos;
						lpos.x = tile.ppos.x;
						lpos.y = tile.ppos.y - (tile.centerRect.top + tile.centerRect.height / 2) + tile.offset.y * 32;

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

	// draw debug grid
	void drawDebug(sf::RenderWindow &window, sf::IntRect clip, float dt) {
		for (int y = clip.top; y < clip.top + clip.height; ++y)
		{
			for (int x = clip.left; x < clip.left + clip.width; ++x)
			{
				// objects
				if (this->map->objs.get(x, y)) {
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
	}

};
