#pragma once

#include "Events.hpp"
#include "GameSystem.hpp"

class TimerSystem : public GameSystem {
public:
	void init() override;
	void update(float dt) override;

};
