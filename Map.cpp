#include "Map.hpp"


Map::Map() {
}

void Map::setSize(unsigned int width, unsigned int height) {
//		this->terrains.setSize(width, height);

	for (int i = 0; i < 6; i++) {
		Layer<int> layer;
		layer.setSize(width, height);
		this->terrains.push_back(layer);
	}

	this->terrainsForTransitions.setSize(width, height);
	this->objs.setSize(width, height);
	this->resources.setSize(width, height);
	this->decors.setSize(width, height);

	this->fogHidden.setSize(width, height);
	this->fogUnvisited.setSize(width, height);

	this->fogHiddenTransitions.setSize(width, height);
	this->fogUnvisitedTransitions.setSize(width, height);

	this->corpses.setSize(width, height);

	this->staticBuildable.setSize(width, height);

	// water & decors
	this->staticPathfinding.setSize(width, height);
	// buildings
	this->pathfinding.setSize(width, height);

	this->units = new Quadtree(0.0, 0.0, width * 32.0f, height * 32.0f, 0, 5);
//	this->units = new Quadtree(0.0, 0.0, width * 32.0f, height * 32.0f, 48);

	this->width = width;
	this->height = height;
}

void Map::markUpdateClear() {
	this->markUpdateTerrainTransitions.clear();
	this->markUpdateFogTransitions.clear();
}
