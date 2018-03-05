#pragma once

#include "GameSystem.hpp"
#include "FlowField.hpp"
#include "Steering.hpp"

#define OBSTACLE_RADIUS 1
#define SURROUNDING_RADIUS 2

#define MIN_VELOCITY 0.01f

class SteeringSystem : public GameSystem {
	Steering<PathfindingObject> steering;

public:
	void update(float dt);
private:
	void updateQuadtrees();
	std::vector<PathfindingObject> getSurroundingSteeringObjects(EntityID currentEnt, float x, float y);
};