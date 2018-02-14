#pragma once

#include <queue>
#include <vector>

#include "Helpers.hpp"

enum TileType {
	Sand,
	SandAlt1,
	SandAlt2,
	Water,
	WaterAlt1,
	WaterAlt2,
	Grass,
	GrassAlt1,
	GrassAlt2,
	Dirt,
	DirtAlt1,
	DirtAlt2,
	Concrete,
	ConcreteAlt1,
	ConcreteAlt2
};

enum TerrainLayer {
	Terrain,
	GrassConcrete,
	SandWater,
	GrassSand,
	ConcreteSand,
	AnyDirt,
};

enum FogType {
	NotVisible = 0,
	Visible = 15
};

class Layer {
public:
	std::vector<int> grid;

	unsigned int width;
	unsigned int height;
	Layer() {}

	Layer(unsigned int width, unsigned int height) : width(width), height(height)  {
		this->fill();
	}

	void setSize(unsigned int w, unsigned int h) {
		this->width = w;
		this->height = h;
		this->fill();
	}

	void fill()
	{
		grid.clear();
		while (grid.size() < width * height) {
			grid.push_back(0);
		}
	}

	void clear() {
		for (int i = 0; i < grid.size(); i++) {
			grid[i] = 0;
		}
	}

	inline int index(int x, int y) const { return x + width * y; }

	int get(int x, int y) const {
		return grid[this->index(x, y)];
	}

	void set(int x, int y, EntityID ent) {
		grid[this->index(x, y)] = ent;
	}

	void del(int x, int y) {
		grid[this->index(x, y)] = 0;
	}

};


class EntityLayer {
public:
	std::vector<EntityID> entitiesGrid;

	unsigned int width;
	unsigned int height;
	EntityLayer() {}

	EntityLayer(unsigned int width, unsigned int height) : width(width), height(height)  {
		this->fill();
	}

	void setSize(unsigned int w, unsigned int h) {
		this->width = w;
		this->height = h;
		this->fill();
	}

	void fill()
	{
		entitiesGrid.clear();
		while (entitiesGrid.size() < width * height) {
			entitiesGrid.push_back(0);
		}
	}

	void clear() {
		for (int i = 0; i < entitiesGrid.size(); i++) {
			entitiesGrid[i] = 0;
		}
	}

	inline int index(int x, int y) const { return x + width * y; }

	EntityID get(int x, int y) const {
		return entitiesGrid[this->index(x, y)];
	}

	void set(int x, int y, EntityID ent) {
		entitiesGrid[this->index(x, y)] = ent;
	}

	void del(int x, int y) {
		entitiesGrid[this->index(x, y)] = 0;
	}

};

enum class FogState {
	Unvisited,
	Hidden,
	InSight
};

class Fog {
public:
	std::vector<FogState> grid;
	unsigned int width;
	unsigned int height;
	Fog() {}

	Fog(unsigned int width, unsigned int height) : width(width), height(height)  {
		this->fill();
	}

	void fill()
	{
		grid.clear();
		while (grid.size() < width * height) {
			grid.push_back(FogState::Unvisited);
		}
	}

	int visited() {
		int visited = 0;
		for (FogState st : grid) {
			if (st != FogState::Unvisited)
				visited++;
		}
		return visited;
	}

	inline int index(int x, int y) const { return x + width * y; }

	FogState get(int x, int y) const {
		return grid[this->index(x, y)];
	}

	void set(int x, int y, FogState st) {
		grid[this->index(x, y)] = st;
	}

};

struct SoundPlay {
	std::string name;
	int priority;
	bool relative;
	sf::Vector2i pos;
};

class SoundPlayCompare
{
public:
	bool operator() (SoundPlay &l, SoundPlay &r)
	{
		return l.priority < r.priority;
	}
};

class Map {
public:

	unsigned int width;
	unsigned int height;

	Layer terrainsForTransitions;

	std::vector<Layer> terrains;

	Layer fogHidden;
	Layer fogUnvisited;

	Layer fogHiddenTransitions;
	Layer fogUnvisitedTransitions;

	EntityLayer objs;
	EntityLayer resources;
	EntityLayer decors;

	EntityLayer effects;

	EntityLayer corpses;

	EntityLayer staticBuildable;

	EntityLayer staticPathfinding;
	EntityLayer pathfinding;

	EntityLayer dynamicPathfinding;
	EntityLayer movingPathfinding;

	// not sure if sound must be there
	std::priority_queue<SoundPlay, std::vector<SoundPlay>, SoundPlayCompare> sounds;

	Map() {
	}

	void setSize(unsigned int width, unsigned int height) {
//		this->terrains.setSize(width, height);

		for (int i = 0; i < 6; i++) {
			Layer layer;
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

		// units
		this->dynamicPathfinding.setSize(width, height);
		this->movingPathfinding.setSize(width, height);

		this->width = width;
		this->height = height;
	}

	inline bool bound(int x, int y) const {
		return (x >= 0 && y >= 0 && x < this->width && y < this->height);
	}

	inline bool pathAvailable(unsigned x, unsigned y) const {
		if (x < width && y < height) // Unsigned will wrap if < 0
		{
			unsigned int idx = x + width * y;
			if (staticPathfinding.entitiesGrid[idx] == 0 && pathfinding.entitiesGrid[idx] == 0)
				return true;
		}
		return false;
	}

	// pathfinding blocking method
	inline bool operator()(unsigned x, unsigned y) const
	{
		return this->pathAvailable(x,y);
	}

};
