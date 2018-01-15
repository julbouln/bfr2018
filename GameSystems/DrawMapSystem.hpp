#pragma once

#include "GameSystem.hpp"

class DrawMapSystem : public GameSystem {
public:
	void draw(sf::RenderWindow &window, float dt) {
		this->drawTileLayer(window, dt);
		this->drawObjLayer(window, dt);
	}

	void drawTileLayer(sf::RenderWindow &window, float dt) {
		TileLayer &layer = this->map->terrains;
		sf::View wview = window.getView();
		sf::FloatRect screenRect(sf::Vector2f(wview.getCenter().x - (wview.getSize().x) / 2, wview.getCenter().y - (wview.getSize().y) / 2) , wview.getSize());

		int mx = screenRect.left / 32.0;
		int my = screenRect.top / 32.0;
		int mw = mx + screenRect.width / 32.0;
		int mh = my + screenRect.height / 32.0;

		mx = mx < 0 ? 0 : mx;
		my = my < 0 ? 0 : my;
		mw = mw > layer.width ? layer.width : mw;
		mh = mh > layer.height ? layer.height : mh;

		for (int y = my; y < mh; ++y)
		{
			for (int x = mx; x < mw; ++x)
			{
				EntityID ent = layer.get(x, y);
				if (ent) {
					Tile &tile = this->vault->registry.get<Tile>(ent);

					sf::Vector2f pos;
					pos.x = tile.ppos.x;
					pos.y = tile.ppos.y;
					pos.x = x * 32;
					pos.y = y * 32;

					tile.sprite.setPosition(pos);

					/* Draw the tile */
					window.draw(tile.sprite);
				}
			}

		}
	}

	void drawObjLayer(sf::RenderWindow &window, float dt) {
		sf::View wview = window.getView();
		sf::FloatRect screenRect(sf::Vector2f(wview.getCenter().x - (wview.getSize().x) / 2, wview.getCenter().y - (wview.getSize().y) / 2) , wview.getSize());

		for (EntityID ent : this->map->entities) {
			Tile &tile = this->vault->registry.get<Tile>(ent);

			sf::Vector2f pos;

//		pos.x = tile.ppos.x - tile.offset.x * 32;
//		pos.y = tile.ppos.y - tile.offset.y * 32;

			pos.x = tile.ppos.x - tile.psize.x / 2 + 16;
			pos.y = tile.ppos.y - tile.psize.y / 2;

			tile.sprite.setPosition(pos);

			sf::FloatRect collider(tile.sprite.getGlobalBounds().left,
			                       tile.sprite.getGlobalBounds().top, 32, 32);

			/* Draw the tile */
			if (screenRect.intersects(collider))
				window.draw(tile.sprite);
		}
	}

};
