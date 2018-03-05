#pragma once

#include "Events.hpp"

#include "GameSystem.hpp"

class TileAnimSystem : public GameSystem {
public:
	void update(float dt);
private:
	void updateStaticSpritesheets(float dt);
	void updateAnimatedSpritesheets(float dt);
};
