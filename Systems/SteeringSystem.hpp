#pragma once

#include "GameSystem.hpp"
#include "FlowField.hpp"
#include "Steering.hpp"

class SteeringSystem : public GameSystem {
	Steering<PathfindingObject> steering;

public:
	void update(float dt);
private:
	void updateQuadtrees();
	std::vector<PathfindingObject> getSurroundingSteeringObjects(EntityID currentEnt, float x, float y);
};