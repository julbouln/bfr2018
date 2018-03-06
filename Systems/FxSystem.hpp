#pragma once

#include "GameSystem.hpp"

class FxSystem : public GameSystem {
	sf::RenderTexture renderTexture;
public:
	void init();
	void update(float dt);
	void draw(sf::RenderWindow &window, sf::IntRect clip, float dt);

	void clear();
};
