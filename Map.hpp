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


class Map {
public:
	std::map<std::string, std::vector<EntityID>> tiles;

	std::vector<EntityID> dirtTransitions;
	std::map<int,int> dirtTransitionsMapping;

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

	std::vector<EntityID> initTile(std::string name, entt::Registry<EntityID> &registry, EntityFactory &factory) {
		std::vector<EntityID> tileVariants;
		for (int i = 0; i < 3; i++) {
			tileVariants.push_back(factory.createTerrain(registry, name, i));
		}
		return tileVariants;
	}

	EntityID randTile(std::string name) {
		int rnd = rand() % 3;
		return tiles[name][rnd];
	}

	void initTiles(entt::Registry<EntityID> &registry, EntityFactory &factory) {
		tiles["sand"] = this->initTile("sand", registry, factory);
		tiles["water"] = this->initTile("water", registry, factory);
		tiles["grass"] = this->initTile("grass", registry, factory);
		tiles["dirt"] = this->initTile("dirt", registry, factory);
		tiles["concrete"] = this->initTile("concrete", registry, factory);
	}

	void initTransitions(entt::Registry<EntityID> &registry, EntityFactory &factory) {
		for (int i = 0; i < 20; i++) {
			dirtTransitions.push_back(factory.createTerrain(registry, "dirt_transition", i));
		}

		dirtTransitionsMapping[1]=2;
		dirtTransitionsMapping[2]=0;
		dirtTransitionsMapping[3]=4;
		dirtTransitionsMapping[4]=1;
		dirtTransitionsMapping[5]=6;
		dirtTransitionsMapping[8]=3;
		dirtTransitionsMapping[10]=7;
		dirtTransitionsMapping[12]=5;

		// miss

	}



	void updateTransitions() {
		for (int x = 0; x < width; x++) {
			for (int y = 0; y < height; y++) {
				int bitmask = 0;
				if (terrains.get(x, y) != tiles["dirt"][0]) {
					if (this->bound(x, y - 1))
						bitmask += 1 * ((terrains.get(x, y - 1) == tiles["dirt"][0]) ? 1 : 0);
					if (this->bound(x - 1, y))
						bitmask += 2 * ((terrains.get(x - 1, y) == tiles["dirt"][0]) ? 1 : 0);
					if (this->bound(x + 1, y))
						bitmask += 4 * ((terrains.get(x + 1, y) == tiles["dirt"][0]) ? 1 : 0);
					if (this->bound(x, y + 1))
						bitmask += 8 * ((terrains.get(x, y + 1) == tiles["dirt"][0]) ? 1 : 0);

//					if (bitmask)
//						std::cout << "BITMASK " << x << "x" << y << " : " << bitmask << std::endl;


				}
				
				if(bitmask) {
					int trans = 0;
					if(dirtTransitionsMapping.count(bitmask) > 0) {
						trans=dirtTransitions[dirtTransitionsMapping[bitmask]];
						transitions.set(x, y, trans);
					}
					else
						transitions.set(x, y, 0);
//						trans=dirtTransitions[bitmask];

				}
				else
					transitions.set(x, y, 0);

			}
		}
	}

	void generate(entt::Registry<EntityID> &registry, EntityFactory &factory, unsigned int width, unsigned int height) {
		terrains.width = width;
		terrains.height = height;

		transitions.width = width;
		transitions.height = height;

		objs.width = width;
		objs.height = height;

		resources.width = width;
		resources.height = height;

		this->width = width;
		this->height = height;

		float random_w = ((float) rand()) / (float) RAND_MAX;
		float random_h = ((float) rand()) / (float) RAND_MAX;

		SimplexNoise simpl(width / 16.0, height / 16.0, 2.0, 0.5);

		terrains.fill();
		transitions.fill();
		objs.fill();
		resources.fill();

		for (float y = 0; y < height; y++) {
			for (float x = 0; x < width; x++) {
				float res = (simpl.fractal(64, x / (width) + random_w * width, y / (height) + random_h * height));

				EntityID t;
//				t = this->randTile("dirt");
				t = tiles["dirt"][0];
				/*
								if (res > -0.5) {
									t = tiles["dirt"];
								} else {
									t = tiles["water"];
								}
				*/
				terrains.set(x, y, t);

				if (res > 0.6 && res < 0.65) {
					float rnd = ((float) rand()) / (float) RAND_MAX;
					if (rnd > 0.5) {
						factory.plantResource(registry, ResourceType::Nature, x, y);
					} else {
						factory.plantResource(registry, ResourceType::Pollution, x, y);
					}
				}
			}
		}


	}


};
