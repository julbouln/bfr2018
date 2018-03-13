#pragma once

#include "GameSystem.hpp"

#include "third_party/dbscan/dbscan.h"

#define RANGE_RADIUS 32.0f

class CombatSystem : public GameSystem {
public:
	void init() override;
	void update(float dt) override;

	void updateFront(float dt);

// signals
	void receive(const TimerLooped &event);
	void receive(const TimerStarted &event);
	void receive(const TimerEnded &event);

private:
	void attacking(EntityID entity);
	bool posInRange(Tile & tile, sf::Vector2f & destPos, int dist, int maxDist);
	bool ennemyInRange(Tile & tile, Tile & destTile, int dist, int maxDist);
	
};