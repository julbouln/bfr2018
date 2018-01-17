#pragma once


class TileLayer {
	std::vector<EntityID> entitiesGrid;
public:
	unsigned int width;
	unsigned int height;
	TileLayer() {}

	TileLayer(unsigned int width, unsigned int height) : width(width), height(height)  {
		this->fill();
	}

	void fill()
	{
		entitiesGrid.clear();
		while (entitiesGrid.size() < width * height) {
			entitiesGrid.push_back(0);
		}
	}

	int index(int x, int y) const { return x + width * y; }

	EntityID get(int x, int y) {
		return entitiesGrid[this->index(x, y)];
	}

	void set(int x, int y, EntityID ent) {
		entitiesGrid[this->index(x, y)] = ent;
	}

	void del(int x, int y) {
		entitiesGrid[this->index(x, y)] = 0;
	}
};

class ObjLayer {
public:
	std::vector<EntityID> entitiesGrid;

	unsigned int width;
	unsigned int height;
	ObjLayer() {}

	ObjLayer(unsigned int width, unsigned int height) : width(width), height(height)  {
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

	int index(int x, int y) const { return x + width * y; }

	EntityID get(int x, int y) {
		return entitiesGrid[this->index(x, y)];
	}

	void set(int x, int y, EntityID ent) {
		entitiesGrid[this->index(x, y)] = ent;
	}

	void del(int x, int y) {
		entitiesGrid[this->index(x, y)] = 0;
	}

	void move(int fromX, int fromY, int toX, int toY) {
		EntityID ent = this->get(fromX, fromY);
		this->set(toX, toY, ent);
		this->del(fromX, fromY);
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

	int index(int x, int y) const { return x + width * y; }

	FogState get(int x, int y) {
		return grid[this->index(x, y)];
	}

	void set(int x, int y, FogState st) {
		grid[this->index(x, y)] = st;
	}

};

class Map {
public:

	unsigned int width;
	unsigned int height;

	TileLayer terrains;
	TileLayer transitions;

	ObjLayer objs;
	ObjLayer resources;

	std::vector<EntityID> entities;

	void addEntity(EntityID entity) {
		entities.push_back(entity);
	}

	void clearEntities() {
		entities.clear();
	}

	Map() {
	}


	bool bound(int x, int y) {
		if (x >= 0 && y >= 0 && x < this->width && y < this->height)
			return true;
		else
			return false;

	}

	inline bool operator()(unsigned x, unsigned y) const
	{
		if (x < width && y < height) // Unsigned will wrap if < 0
		{
//			std::cout << "PATHFINDING "<< x << "x" << y << " " << objs.entitiesGrid[x + width * y] << std::endl;
			if (objs.entitiesGrid[x + width * y] == 0)
				return true;
		}
		return false;
	}

};
