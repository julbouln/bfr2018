#pragma once

#include "GameSystem.hpp"

#include "third_party/dbscan/dbscan.h"

#define RANGE_RADIUS 32.0f

class CombatSystem : public GameSystem {
public:
	void init() override;
	void updateFront(float dt);
	void update(float dt);

// signals
	void receive(const AnimationFrameChanged &event);

private:
	bool posInRange(Tile & tile, sf::Vector2f & destPos, int dist, int maxDist);
	bool ennemyInRange(Tile & tile, Tile & destTile, int dist, int maxDist);
	
};