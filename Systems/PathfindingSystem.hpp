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
#ifndef PATHFINDING_FLOWFIELD
	JPS::Searcher<Map> *search;
#endif

public:
	FlowFieldPathFind flowFieldPathFind;
	Steering<PathfindingObject> steering;

	PathfindingSystem();
	~PathfindingSystem();

	void init();

	void updateSteering(float dt);
	void update(float dt);

private:
	void updatePathfindingLayer(float dt);
	void updateQuadtrees();
	std::vector<PathfindingObject> getSurroundingSteeringObjects(EntityID currentEnt, float x, float y);

	void testDbscan();

};