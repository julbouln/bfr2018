#pragma once

#include "GameSystem.hpp"

class DrawMapSystem : public GameSystem {
public:
	std::vector<EntityID> entitiesDrawList;

	bool showDebugLayer;

	DrawMapSystem() {
		this->showDebugLayer = false;
	}

	void draw(sf::RenderWindow &window, sf::IntRect clip, float dt) {
		this->drawTileLayers(window, clip, dt);

		this->drawObjLayer(window, clip, dt);
		if (showDebugLayer)
			this->drawDebug(window, clip, dt);

		this->drawLayer(window, this->map->fogHidden, clip, dt, sf::Color(0x00, 0x00, 0x00, 0x7f));
		this->drawLayer(window, this->map->fogUnvisited, clip, dt, sf::Color(0x00, 0x00, 0x00));
		this->drawLayer(window, this->map->fogHiddenTransitions, clip, dt, sf::Color(0x00, 0x00, 0x00, 0x7f));
		this->drawLayer(window, this->map->fogUnvisitedTransitions, clip, dt, sf::Color(0x00, 0x00, 0x00));
	}

	void drawSpriteWithShader(sf::RenderTarget & target, sf::Sprite & sprite, std::string shaderName, ShaderOptions & options) {
		sf::Shader *shader = this->vault->factory.shrManager.getRef(shaderName);
		applyShaderOptions(shader, options);
		target.draw(sprite, shader);
	}

	void drawLayer(sf::RenderTarget & target, Layer & layer, sf::IntRect clip, float dt, sf::Color colorVariant = sf::Color(0xff, 0xff, 0xff)) {
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

	void drawTileLayers(sf::RenderTarget & target, sf::IntRect clip, float dt) {
		this->drawLayer(target, this->map->terrains, clip, dt);
		for (Layer &transitionLayer : this->map->transitions) {
			this->drawLayer(target, transitionLayer, clip, dt);
		}

		this->drawLayer(target, this->map->corpses, clip, dt);
	}

	inline bool clipped(sf::IntRect & clip, sf::Vector2i const & p) const {
		return (p.x >= clip.left && p.x <= clip.left + clip.width &&
		        p.y >= clip.top && p.y <= clip.top + clip.height);
	}

	// reduce object list to visible entities
	void updateObjsDrawList(sf::RenderWindow & window, sf::IntRect clip, float dt) {

		this->entitiesDrawList.clear();

		// resources draw list
		auto resView = this->vault->registry.persistent<Tile, Resource>();

		for (EntityID entity : resView) {
			Tile &tile = resView.get<Tile>(entity);

			for (sf::Vector2i const &p : this->tileSurface(tile)) {
				if (this->map->fogHidden.get(p.x, p.y) == 0 && this->map->fogUnvisited.get(p.x, p.y) == 0) {
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
				if (!obj.mapped || (this->map->fogHidden.get(p.x, p.y) == 0 && this->map->fogUnvisited.get(p.x, p.y) == 0)) {
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
				if ((this->map->fogHidden.get(p.x, p.y) == 0 && this->map->fogUnvisited.get(p.x, p.y) == 0)) {
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

	void drawObjLayer(sf::RenderWindow & window, sf::IntRect clip, float dt) {
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
	void drawDebug(sf::RenderWindow & window, sf::IntRect clip, float dt) {
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
