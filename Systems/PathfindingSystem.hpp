#pragma once

#include "third_party/JPS.h"
#include "third_party/dbscan/dbscan.h"

#include "GameSystem.hpp"
#include "FlowField.hpp"
#include "Steering.hpp"

#define PATHFINDING_MAX_NO_PATH 8

#define MIN_VELOCITY 0.01f

class PathfindingSystem : public GameSystem {
public:
	FlowFieldPathFind flowFieldPathFind;

	PathfindingSystem();
	~PathfindingSystem();

	void init() override;
	void update(float dt) override;

private:
	void updatePathfindingLayer(float dt);

	void testDbscan();

};