#pragma once

#include "third_party/JPS.h"
#include "third_party/dbscan/dbscan.h"

#include "GameSystem.hpp"
#include "FlowField.hpp"
#include "Steering.hpp"

#define PATHFINDING_MAX_NO_PATH 8
#define OBSTACLE_RADIUS 1
#define SURROUNDING_RADIUS 2

#define MIN_VELOCITY 0.01f

class PathfindingSystem : public GameSystem {
public:
	FlowFieldPathFind flowFieldPathFind;

	PathfindingSystem();
	~PathfindingSystem();

	void init();
	void update(float dt);

private:
	void updatePathfindingLayer(float dt);

	void testDbscan();

};