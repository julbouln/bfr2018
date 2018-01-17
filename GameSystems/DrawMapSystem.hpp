#pragma once

#include "GameSystem.hpp"

class DrawMapSystem : public GameSystem {
public:
	sf::Shader colorSwap;
	sf::Shader pixelation;

	std::vector<EntityID> entitiesDrawList;

	DrawMapSystem() {
		if (!colorSwap.loadFromFile("defs/new/shaders/color_swap.frag", sf::Shader::Fragment))
		{
			// erreur...
		}
		if (!pixelation.loadFromFile("defs/new/shaders/pixelation.frag", sf::Shader::Fragment))
		{
			// erreur...
		}

	}

	void draw(sf::RenderWindow &window, float dt) {
		this->drawTileLayer(window, this->map->terrains, dt);
		this->drawTileLayer(window, this->map->transitions, dt);
		this->drawObjLayer(window, dt);
		this->drawDebug(window, dt);
	}

	sf::IntRect viewClip(sf::RenderWindow &window) {
		sf::View wview = window.getView();
		sf::FloatRect screenRect(sf::Vector2f(wview.getCenter().x - (wview.getSize().x) / 2, wview.getCenter().y - (wview.getSize().y) / 2) , wview.getSize());

		int mx = screenRect.left / 32.0;
		int my = screenRect.top / 32.0;
		int mw = mx + screenRect.width / 32.0;
		int mh = my + screenRect.height / 32.0;

		mx = mx < 0 ? 0 : mx;
		my = my < 0 ? 0 : my;
		mw = mw > this->map->width ? this->map->width : mw;
		mh = mh > this->map->height ? this->map->height : mh;

		return sf::IntRect(sf::Vector2i(mx, my), sf::Vector2i(mw - mx, mh - my));
	}

	void drawTileLayer(sf::RenderWindow &window, TileLayer &layer, float dt, sf::Color colorVariant = sf::Color(0xff,0xff,0xff)) {
		sf::IntRect clip = this->viewClip(window);

//		std::cout << "clip "<<clip.left<<"x"<<clip.top<<":"<<clip.width<<"x"<<clip.height<<std::endl;

		for (int y = clip.top; y < clip.top + clip.height; ++y)
		{
			for (int x = clip.left; x < clip.left + clip.width; ++x)
			{
//				std::cout << x<<"x"<<y<< " ";
				EntityID ent = layer.get(x, y);
				if (ent) {
					Tile &tile = this->vault->registry.get<Tile>(ent);

					sf::Vector2f pos;
					pos.x = tile.ppos.x;
					pos.y = tile.ppos.y;
					pos.x = x * 32;
					pos.y = y * 32;

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

	void updateObjsDrawList(sf::RenderWindow &window, float dt) {
		sf::IntRect clip = this->viewClip(window);

		this->entitiesDrawList.clear();
		auto resView = this->vault->registry.persistent<Tile, Resource>();

		for (EntityID entity : resView) {
			Tile &tile = resView.get<Tile>(entity);

			for (sf::Vector2i p : this->tileSurface(tile)) {
				if (p.x >= clip.left && p.x <= clip.left + clip.width &&
				        p.y >= clip.top && p.y <= clip.top + clip.height)
					this->entitiesDrawList.push_back(entity);
			}
		}

		auto view = this->vault->registry.persistent<Tile, GameObject>();

		for (EntityID entity : view) {
			Tile &tile = view.get<Tile>(entity);

			for (sf::Vector2i p : this->tileSurface(tile)) {
				if (p.x >= clip.left && p.x <= clip.left + clip.width &&
				        p.y >= clip.top && p.y <= clip.top + clip.height)
					this->entitiesDrawList.push_back(entity);
			}
		}

		// sort by EntityID and uniq
		std::sort(this->entitiesDrawList.begin(), this->entitiesDrawList.end());
		auto last = std::unique(this->entitiesDrawList.begin(), this->entitiesDrawList.end());
		this->entitiesDrawList.erase(last, this->entitiesDrawList.end());

		// sort by position
		std::sort( this->entitiesDrawList.begin( ), this->entitiesDrawList.end( ), [this ]( const auto & lhs, const auto & rhs )
		{
			Tile &lht = vault->registry.get<Tile>(lhs);
			Tile &rht = vault->registry.get<Tile>(rhs);
			return (lht.ppos.y + (lht.centerRect.top + lht.centerRect.height)/2 < rht.ppos.y + (rht.centerRect.top+rht.centerRect.height)/2);
		});
	}

	void drawObjLayer(sf::RenderWindow &window, float dt) {
		this->updateObjsDrawList(window, dt);
		sf::View wview = window.getView();
		sf::FloatRect screenRect(sf::Vector2f(wview.getCenter().x - (wview.getSize().x) / 2, wview.getCenter().y - (wview.getSize().y) / 2) , wview.getSize());

		for (EntityID ent : this->entitiesDrawList) {
			if (this->vault->registry.valid(ent)) { // DIRTY
				Tile &tile = this->vault->registry.get<Tile>(ent);

				sf::Vector2f pos;

				pos.x = tile.ppos.x - (tile.centerRect.left + tile.centerRect.width / 2) + 16 + tile.offset.x * 32;
				pos.y = tile.ppos.y - (tile.centerRect.top + tile.centerRect.height / 2) + 16 + tile.offset.y * 32;

				tile.sprite.setPosition(pos);

				sf::FloatRect collider(tile.sprite.getGlobalBounds().left,
				                       tile.sprite.getGlobalBounds().top, 32, 32);

				colorSwap.setParameter("texture", sf::Shader::CurrentTexture);
				colorSwap.setParameter("color1", sf::Color(3, 255, 205));
				colorSwap.setParameter("replace1", sf::Color(117, 122, 223));
				colorSwap.setParameter("color2", sf::Color(0, 235, 188));
				colorSwap.setParameter("replace2", sf::Color(90, 94, 172));

				window.draw(tile.sprite, &colorSwap);
			}
		}
	}

	void drawDebug(sf::RenderWindow &window, float dt) {
		sf::IntRect clip = this->viewClip(window);

		// draw debug grid
		for (int y = clip.top; y < clip.top + clip.height; ++y)
		{
			for (int x = clip.left; x < clip.left + clip.width; ++x)
			{
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
