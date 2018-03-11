#pragma once

#include "GameSystem.hpp"

class MinimapSystem : public GameSystem {
	sf::Uint8* pixels;
	sf::Texture texture;

	sf::Sprite sprite;

public:
	sf::FloatRect rect;
	float size;

	MinimapSystem();
	~MinimapSystem();

	void init() override;
	void update(EntityID playerEnt, float dt);
	void draw(sf::RenderWindow &window, float dt);
	void drawClip(sf::RenderWindow &window, sf::View &view, sf::IntRect clip, float dt);

private:
	void drawFrame(sf::RenderWindow &window);
	inline void setMinimapPixel(int idx, sf::Color color) {
		pixels[4 * idx] = color.r;
		pixels[4 * idx + 1] = color.g;
		pixels[4 * idx + 2] = color.b;
		pixels[4 * idx + 3] = color.a;
	}

};
