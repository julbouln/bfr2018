#pragma once

#include "GameSystem.hpp"

class FxSystem : public GameSystem {
public:
	void update(float dt);
	void draw(sf::RenderWindow &window, sf::IntRect clip, float dt);

	void clear();
};
