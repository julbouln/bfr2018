#pragma once

#include "GameSystem.hpp"

class DrawMapSystem : public GameSystem {
public:
	sf::Shader colorSwap;
	sf::Shader pixelation;

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

//	pixelation.setParameter("texture", sf::Shader::CurrentTexture);
//	pixelation.setParameter("amount", 32.0);
//				window.draw(tile.sprite,&pixelation);
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
			if (screenRect.intersects(collider)) {
/*				pixelation.setParameter("texture", sf::Shader::CurrentTexture);
				window.draw(tile.sprite,&pixelation);
*/

				colorSwap.setParameter("texture", sf::Shader::CurrentTexture);
				colorSwap.setParameter("color1", sf::Color(3,255,205));
				colorSwap.setParameter("replace1", sf::Color(117,122,223));
				colorSwap.setParameter("color2", sf::Color(0,235,188));
				colorSwap.setParameter("replace2", sf::Color(90,94,172));

				window.draw(tile.sprite,&colorSwap);
				
			}
		}
	}

	void drawDebug(sf::RenderWindow &window, float dt) {

		// draw debug grid
		for (int y = 0; y < this->map->height; ++y)
		{
			for (int x = 0; x < this->map->width; ++x)
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
