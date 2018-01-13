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
	std::vector<EntityID> entities;

	unsigned int width;
	unsigned int height;
	ObjLayer() {}

	ObjLayer(unsigned int width, unsigned int height) : width(width), height(height)  {
		this->fill();
	}

	void fill()
	{
		entities.clear();
		entitiesGrid.clear();
		while (entitiesGrid.size() < width * height) {
			entitiesGrid.push_back(0);
		}
	}

	int index(int x, int y) const { return x + width * y; }

	void add(EntityID entity) {
		entities.push_back(entity);
	}

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


class Map {
public:

	std::map<std::string,EntityID> tiles;

	unsigned int width;
	unsigned int height;

	TileLayer terrains;

	ObjLayer objs;

	Map() {
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

	void initTiles(entt::Registry<EntityID> &registry, EntityFactory &factory) {
		tiles["sand"]=factory.createTerrain(registry, "sand", 0, 0);
		tiles["water"]=factory.createTerrain(registry, "water", 0, 0);
		tiles["grass"]=factory.createTerrain(registry, "grass", 0, 0);
		tiles["dirt"]=factory.createTerrain(registry, "dirt", 0, 0);
		tiles["concrete"]=factory.createTerrain(registry, "concrete", 0, 0);
	}

	void generate(entt::Registry<EntityID> &registry, EntityFactory &factory, unsigned int width, unsigned int height) {
		terrains.width = width;
		terrains.height = height;

		objs.width = width;
		objs.height = height;

		this->width = width;
		this->height = height;

		float random_w = ((float) rand()) / (float) RAND_MAX;
		float random_h = ((float) rand()) / (float) RAND_MAX;

		SimplexNoise simpl(width / 16.0, height / 16.0, 2.0, 0.5);

		terrains.fill();

		for (float y = 0; y < height; y++) {
			for (float x = 0; x < width; x++) {
				float res = (simpl.fractal(64, x / (width) + random_w * width, y / (height) + random_h * height));

				EntityID t;
				if (res > -0.5) {
//					std::cout << " " << x << "x" << y << " " << "dirt" << std::endl;
//					t = entityFactory.createTerrain(registry, "dirt", x, y);
					t = tiles["dirt"];
				} else {
//					std::cout << " " << x << "x" << y << " " << "water" << std::endl;
					t = tiles["water"];
//					t = entityFactory.createTerrain(registry, "water", x, y);
				}
				terrains.set(x, y, t);
			}
		}

		objs.fill();
	}


};
