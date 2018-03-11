#pragma once

#include <queue>
#include <vector>
#include <set>

#include "Entity.hpp"
#include "Helpers.hpp"
#include "Quadtree.hpp"

#define VECTOR_LAYER

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

#ifdef VECTOR_LAYER
template <typename T, T Empty = 0>
class Layer {
public:
	std::vector<T> grid;

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
		grid.reserve(width * height);
		while (grid.size() < width * height) {
			grid.push_back(Empty);
		}
	}

	void clear() {
		for (int i = 0; i < grid.size(); ++i) {
			grid[i] = Empty;
		}
	}

	int size() {
		return grid.size();
	}

	inline int index(int x, int y) const { return x + width * y; }

	T get(int x, int y) const {
		return grid[this->index(x, y)];
	}

	void set(int x, int y, T ent) {
		grid[this->index(x, y)] = ent;
	}
};

#else
template <typename T, T Empty = 0>
class Layer {
public:
	T* grid;

	unsigned int width;
	unsigned int height;
	Layer() {
//		grid = nullptr;
	}

	Layer(unsigned int width, unsigned int height) : width(width), height(height)  {
		this->fill();
	}

	~Layer() {
//		if(grid)
//			delete[] grid;
	}

	void setSize(unsigned int w, unsigned int h) {
		this->width = w;
		this->height = h;
		this->fill();
	}

	void fill()
	{
		grid = new T[width * height]();
		this->clear();
	}

	void clear() {
		for (int i = 0; i < width * height; ++i) {
			grid[i] = Empty;
		}
	}

	int size() {
		return width * height;
	}

	inline int index(int x, int y) const { return x + width * y; }

	T get(int x, int y) const {
		return grid[this->index(x, y)];
	}

	void set(int x, int y, T ent) {
		grid[this->index(x, y)] = ent;
	}
};
#endif

enum class FogState {
	Unvisited,
	Hidden,
	InSight
};

class Fog : public Layer<FogState, FogState::Unvisited> {
public:
	int visited() {
		int visited = 0;
//		for (FogState st : grid) {
		for (int i = 0; i < width * height; ++i) {
			FogState &st = grid[i];
			if (st != FogState::Unvisited)
				visited++;
		}
		return visited;
	}
};

class PathfindingObject;

class Map {
public:

	unsigned int width;
	unsigned int height;

	Layer<int> terrainsForTransitions;

	std::vector<Layer<int>> terrains;

	Layer<int> fogHidden;
	Layer<int> fogUnvisited;

	Layer<int> fogHiddenTransitions;
	Layer<int> fogUnvisitedTransitions;

	Layer<EntityID> objs;
	Layer<EntityID> resources;
	Layer<EntityID> decors;

	Layer<EntityID> effects;

	Layer<EntityID> water;
	Layer<EntityID> corpses;

	Layer<EntityID> staticBuildable;

	Layer<EntityID> staticPathfinding;
	Layer<EntityID> pathfinding;

	Quadtree<PathfindingObject>* units;

	// transitions calculation optimization
	// maintain a list of position to update instead of updating every transitions
	std::set<sf::Vector2i, CompareVector2i> markUpdateTerrainTransitions;
	std::set<sf::Vector2i, CompareVector2i> markUpdateFogTransitions;

	Map();

	void setSize(unsigned int width, unsigned int height);

	void markUpdateClear();


	inline bool bound(int x, int y) const {
		return (x >= 0 && y >= 0 && x < this->width && y < this->height);
	}

	inline bool pathAvailable(unsigned x, unsigned y) const {
		if (x < width && y < height) // Unsigned will wrap if < 0
		{
			unsigned int idx = x + width * y;
			if (staticPathfinding.grid[idx] == 0 && pathfinding.grid[idx] == 0)
				return true;
		}
		return false;
	}

	inline bool positionAvailable(unsigned x, unsigned y) const {
		if (x < width && y < height) // Unsigned will wrap if < 0
		{
			unsigned int idx = x + width * y;
			if (staticPathfinding.grid[idx] == 0 && pathfinding.grid[idx] == 0 && objs.grid[idx] == 0)
				return true;
		}
		return false;
	}

// pathfinding blocking method
	inline bool operator()(unsigned x, unsigned y) const
	{
		return this->pathAvailable(x, y);
	}

};
