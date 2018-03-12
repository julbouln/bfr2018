#pragma once

#include "Events.hpp"
#include "GameSystem.hpp"

class TileAnimSystem : public GameSystem {
public:
	void init() override;
	void update(float dt) override;

	// signals
	void receive(const StateChanged &event);

private:
	void updateStaticSpritesheets(float dt);
	void updateAnimatedSpritesheets(float dt);
};
