#pragma once

#include "GameSystem.hpp"

class SoundSystem : public GameSystem {
	std::list<sf::Sound> playing;
public:
	void update(float dt);
};