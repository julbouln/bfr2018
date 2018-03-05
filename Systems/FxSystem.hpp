#pragma once

#include "GameSystem.hpp"

class FxSystem : public GameSystem {
public:
	void update(float dt);
	void clear();
	void draw(sf::RenderWindow &window, sf::IntRect clip, float dt);
};
