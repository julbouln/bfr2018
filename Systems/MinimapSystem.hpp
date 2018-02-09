#pragma once

#include "GameSystem.hpp"

class MinimapSystem : public GameSystem {
	sf::Uint8* pixels;
	sf::Texture texture;

	sf::Sprite sprite;

public:
	sf::FloatRect rect;
	float size;

	MinimapSystem() {
		pixels = nullptr;
	}

	~MinimapSystem() {
		if (pixels)
			delete pixels;
	}

	sf::Texture &createTexture() {
		texture.create(this->map->width, this->map->height);
		pixels = new sf::Uint8[this->map->width * this->map->height * 4];
		return texture;
	}

	void init(sf::Vector2f pos, float s) {
		texture.create(this->map->width, this->map->height);
		pixels = new sf::Uint8[this->map->width * this->map->height * 4];
		rect = sf::FloatRect(pos.x, pos.y, s, s);
		size = s;
		sprite.setTexture(texture);
	}

	inline void setMinimapPixel(int idx, sf::Color color) {
		pixels[4 * idx] = color.r;
		pixels[4 * idx + 1] = color.g;
		pixels[4 * idx + 2] = color.b;
		pixels[4 * idx + 3] = color.a;
	}

	void update(EntityID playerEnt, float dt) {
		Player &player = this->vault->registry.get<Player>(playerEnt);

		int idx = 0;
		for (int y = 0; y < this->map->height; ++y) {
			for (int x = 0; x < this->map->width; ++x) {
				FogState fogSt = player.fog.get(x, y);
				if (fogSt != FogState::Unvisited) {
					if (fogSt != FogState::Hidden) {
						this->setMinimapPixel(idx, sf::Color(0x67, 0x51, 0x0e, 0xff));
					} else {
						this->setMinimapPixel(idx, sf::Color(0x63, 0x4d, 0x0a, 0x7f));
					}

					if (fogSt != FogState::Hidden) {
						EntityID objEnt = this->map->objs.get(x, y);
						if (objEnt) {
							GameObject &obj = this->vault->registry.get<GameObject>(objEnt);
							Player &objPlayer = this->vault->registry.get<Player>(obj.player);
							this->setMinimapPixel(idx, objPlayer.color);
						}
					}
				} else {
					this->setMinimapPixel(idx, sf::Color::Black);
				}
				++idx;
			}
		}
		texture.update(pixels);
	}

	void draw(sf::RenderWindow &window, float dt) {
		sprite.setPosition(sf::Vector2f(rect.left, rect.top));
		sprite.setScale(sf::Vector2f(this->size / this->map->width, this->size / this->map->height));
		window.draw(sprite);

		this->drawFrame(window);
	}

	void drawClip(sf::RenderWindow &window, sf::View &view, sf::IntRect clip, float dt) {
		sf::IntRect mClip = clip;
		mClip.left = (rect.left + (view.getCenter().x / 32.0 - (this->screenWidth) / 32.0 / 2.0) * (this->size / this->map->width));
		mClip.top = (rect.top + (view.getCenter().y / 32.0 - (this->screenHeight) / 32.0 / 2.0) * (this->size / this->map->height));
		mClip.width = (int)((float)(this->screenWidth / 32.0) * (this->size / this->map->width));
		mClip.height = (int)((float)(this->screenHeight / 32.0) * (this->size / this->map->height));

		// clip rectangle
		sf::RectangleShape clipR;
		sf::Vector2f clipPos(mClip.left, mClip.top);
		clipR.setSize(sf::Vector2f(mClip.width, mClip.height));
		clipR.setFillColor(sf::Color(0x00, 0x00, 0x00, 0x00));
		clipR.setOutlineColor(sf::Color(0xff, 0xff, 0xff, 0xff));
		clipR.setOutlineThickness(1);
		clipR.setPosition(clipPos);
		window.draw(clipR);
	}

private:
	void drawFrame(sf::RenderWindow &window) {
		// frame rectangle
		sf::RectangleShape r;
		sf::Vector2f rPos(rect.left + 1, rect.top + 1);
		r.setSize(sf::Vector2f(size - 2, size - 2));
		r.setFillColor(sf::Color(0x00, 0x00, 0x00, 0x00));
		r.setOutlineColor(sf::Color(0x66, 0x66, 0x66, 0xff));
		r.setOutlineThickness(1);
		r.setPosition(rPos);
		window.draw(r);
	}
};
