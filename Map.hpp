#pragma once

#include <queue>
#include <vector>

class Layer {
public:
	std::vector<EntityID> entitiesGrid;

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
	std::vector<FogState> grid;
public:
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

	FogState get(int x, int y) {
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


struct CompareVector2i
{
	bool operator()(sf::Vector2i a, sf::Vector2i b) const
	{
		if (a.x < b.x)
			return true;
		else if (b.x < a.x)
			return false;
		else
			return a.y < b.y;
	}
};

class Map {
public:

	unsigned int width;
	unsigned int height;

	Layer terrains;
	Layer terrainsForTransitions;

	std::vector<Layer> transitions;

	Layer fogHidden;
	Layer fogUnvisited;

	Layer fogHiddenTransitions;
	Layer fogUnvisitedTransitions;

	Layer objs;
	Layer resources;
	Layer decors;

	Layer effects;

	Layer corpses;

	Layer staticBuildable;

	Layer staticPathfinding;
	Layer pathfinding;

	// not sure if sound must be there
	std::priority_queue<SoundPlay, std::vector<SoundPlay>, SoundPlayCompare> sounds;

	Map() {
	}

	void setSize(unsigned int width, unsigned int height) {
		this->terrains.setSize(width, height);

		for (int i = 0; i < 5; i++) {
			Layer layer;
			layer.setSize(width, height);
			this->transitions.push_back(layer);
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

		this->staticPathfinding.setSize(width, height);
		this->pathfinding.setSize(width, height);

		this->width = width;
		this->height = height;
	}

	bool bound(int x, int y) {
		if (x >= 0 && y >= 0 && x < this->width && y < this->height)
			return true;
		else
			return false;
	}

	// pathfinding blocking method
	inline bool operator()(unsigned x, unsigned y) const
	{
		if (x < width && y < height) // Unsigned will wrap if < 0
		{
//			std::cout << "PATHFINDING "<< x << "x" << y << " " << objs.entitiesGrid[x + width * y] << std::endl;
			if (staticPathfinding.entitiesGrid[x + width * y] == 0 && pathfinding.entitiesGrid[x + width * y] == 0)
				return true;
		}
		return false;
	}

};
