#pragma once

#include "Map.hpp"
#include "third_party/JPS.h"

typedef short int field_t;
typedef signed char dir_t;

const sf::Vector2i DIRECTIONS[] = { sf::Vector2i(1, 0), sf::Vector2i(1, 1), sf::Vector2i(0, 1), sf::Vector2i(-1, 1),
                                    sf::Vector2i(-1, 0), sf::Vector2i(-1, -1), sf::Vector2i(0, -1), sf::Vector2i(1, -1)
                                  };

class Grid {
	Map* map;
public:
	unsigned int width;
	unsigned int height;
	sf::IntRect rect;

	Grid() {
	}

	Grid(Map *m, sf::IntRect rect) {
		this->setGrid(m, rect);
	}

	Grid(Map *m) {
		this->setGrid(m);
	}

	void setGrid(Map *m) {
		map = m;
		rect = sf::IntRect(0, 0, m->width, m->height);
		width = m->width;
		height = m->height;
	}

	void setGrid(Map *m, sf::IntRect rect) {
		map = m;
		rect = rect;
		width = rect.width;
		height = rect.height;
	}

	bool pathAvailable(int x, int y) const {
		return map->pathAvailable(x + rect.left, y + rect.top);
	}

	bool bound(int x, int y) const {
		return map->bound(x + rect.left, y + rect.top) && x < rect.width && y < rect.height;
	}

};

class FlowField {
	std::vector<field_t> _fields;
	std::vector<dir_t> _dir;
	Grid _grid;

public:
	FlowField() {
//		_fields = nullptr;
//		_dir = nullptr;
	}

	void resize() {
		_fields.clear();
		_dir.clear();
		_fields.reserve(_grid.width * _grid.height);
		while (_fields.size() < _grid.width * _grid.height) {
			_fields.push_back(std::numeric_limits<short int>::max());
		}
		_dir.reserve(_grid.width * _grid.height);
		while (_dir.size() < _grid.width * _grid.height) {
			_dir.push_back(-1);
		}
	}

	void setGrid(Grid grid) {

	}

	void setGrid(Map *map, sf::IntRect rect) {
		_grid = Grid(map, rect);
	}

	void setGrid(Map *map) {
		_grid = Grid(map);
//		if (!_fields)
//			_fields = new field_t[_grid->width * _grid->height];
//		if (!_dir)
//			_dir = new dir_t[_grid->width * _grid->height];
	}

	~FlowField() {
//		if (_dir)
//			delete[] _dir;
//		if (_fields)
//			delete[] _fields;
	}

// -------------------------------------------------------------
// Checks whether the index is already in the list
// -------------------------------------------------------------
	bool checkIfContains(unsigned int idx, const std::list<unsigned int>& lst) const {
		std::list<unsigned int>::const_iterator it = lst.begin();
		while (it != lst.end()) {
			if (*it == idx) {
				return true;
			}
			++it;
		}
		return false;
	}

// -------------------------------------------------------------
// get neighbors (N, S, W and E). Will return the indices
// -------------------------------------------------------------
	int getNeighbors(int x, int y, int * ret, int max) {
		int cnt = 0;
		if (_grid.bound(x, y - 1) && _grid.pathAvailable(x, y - 1)) {
			ret[cnt++] = x + (y - 1) * _grid.width;
		}
		if (_grid.bound(x, y + 1) && _grid.pathAvailable(x, y + 1)) {
			ret[cnt++] = x + (y + 1) * _grid.width;
		}
		if (_grid.bound(x - 1, y) && _grid.pathAvailable(x - 1, y)) {
			ret[cnt++] = x - 1 + y * _grid.width;
		}
		if (_grid.bound(x + 1, y) && _grid.pathAvailable(x + 1, y)) {
			ret[cnt++] = x + 1 + y * _grid.width;
		}
		return cnt;
	}

// -------------------------------------------------------------
// find the index of the neighbor with the lowest cost
// -------------------------------------------------------------
	int findLowestCost(int x, int y) {
		int m = std::numeric_limits<short int>::max();
		int ret = 14;
		for (int i = 0; i < 8; ++i) {
			sf::Vector2i c = sf::Vector2i(x, y) + DIRECTIONS[i];
			if (_grid.pathAvailable(c.x, c.y)) {
				int idx = c.x + c.y * _grid.width;
				if (_fields[idx] < m) {
					ret = i;
					m = _fields[idx];
				}
			}
		}
		return ret;
	}

// -------------------------------------------------------------
// reset fields and directions
// -------------------------------------------------------------
	void resetFields() {
		this->resize();
		/*
				int total = _grid.width * _grid.height;
				for (int i = 0; i < total; ++i) {
					_fields[i] = std::numeric_limits<short int>::max();
					_dir[i] = -1;
				}
				*/
	}

// -------------------------------------------------------------
// build the flow field
// -------------------------------------------------------------
	void build(const sf::Vector2i & end) {
		// simple Dijstra flood fill first
		unsigned int targetID = end.y * _grid.width + end.x;
//		std::cout << "BUILD "<<_grid.width<< "x"<<_grid.height<< " "<<targetID<<std::endl;
		resetFields();
		std::list<unsigned int> openList;
		_fields[targetID] = 0;
		openList.push_back(targetID);
		int neighbors[4];
		while (openList.size() > 0)	{
			unsigned currentID = openList.front();
			openList.pop_front();
			unsigned short currentX = currentID % _grid.width;
			unsigned short currentY = currentID / _grid.width;
			int neighborCount = getNeighbors(currentX, currentY, neighbors, 4);
//			std::cout << "GET neighbors "<<currentID << " "<<currentX<<"x"<<currentY<<" "<<neighborCount<<std::endl;
			for (int i = 0; i < neighborCount; ++i) {
				unsigned int endNodeCost = _fields[currentID] + 1;
				if (endNodeCost < _fields[neighbors[i]]) {
					if (!checkIfContains(neighbors[i], openList)) {
//						std::cout << "ADD neighbor "<<neighbors[i]<<std::endl;
						openList.push_back(neighbors[i]);
					}
					_fields[neighbors[i]] = endNodeCost;
				}
			}

		}
		// now calculate the directions
		for (int x = 0; x < _grid.width; ++x) {
			for (int y = 0; y < _grid.height; ++y) {
				if (_grid.pathAvailable(x, y)) {
					_dir[x + _grid.width * y] = findLowestCost(x, y);
				}
				else {
					_dir[x + _grid.width * y] = 16;
				}
			}
		}
//		std::cout << "FlowField: built " << end.x << "x" << end.y << std::endl;
	}

// -------------------------------------------------------------
// get direction
// -------------------------------------------------------------
	int get(int x, int y) const {
		int idx = x + y * _grid.width;
//		std::cout << "FlowField: get " << x << "x" << y << " : " << idx << " " << _dir[idx] << std::endl;
		return _dir[idx];
	}

// -------------------------------------------------------------
// get cost of cell
// -------------------------------------------------------------
	int getCost(int x, int y) const {
		int idx = x + y * _grid.width;
		return _fields[idx];
	}

// -------------------------------------------------------------
// get the next field based on the direction of the current cell
// -------------------------------------------------------------
	sf::Vector2i next(const sf::Vector2i & current) {
		int dir = get(current.x, current.y);
		return current + DIRECTIONS[dir];
	}

};


#define PER_SECTOR 12

struct BorderPosition {
	int x;
	int y;
	int partIdx;
};

class FlowFields {
	Map *map;
	sf::Clock clock;
public:
	JPS::Searcher<Map> *search;

	unsigned int width;
	unsigned int height;
	std::vector<std::vector<FlowField>> fields;

	FlowFields() {
	}

	void update(int x, int y) {
		int idx = x + y * this->width;
		clock.restart();
		sf::Time elapsed1 = clock.getElapsedTime();
		for (int gy = 0; gy < PER_SECTOR; ++gy) {
			for (int gx = 0; gx < PER_SECTOR; ++gx) {
//						std::cout << "BUILD FIELD " << x << "x" << y << " : " << gx << "x" << gy << " "<<map->pathAvailable(x*PER_SECTOR+gx,y*PER_SECTOR+gy) << std::endl;
				FlowField field;
				field.setGrid(map, sf::IntRect(x * PER_SECTOR, y * PER_SECTOR, PER_SECTOR, PER_SECTOR));
				fields[idx].push_back(field);
				fields[idx].back().build(sf::Vector2i(gx, gy));
			}
		}

		sf::Time elapsed2 = clock.getElapsedTime();
		std::cout << "TIME " <<  x * PER_SECTOR << "x" << y * PER_SECTOR << " " << x << "x" << y << " " << (elapsed2 - elapsed1).asSeconds() << std::endl;

	}

	void init(Map *map) {
		search = new JPS::Searcher<Map>(*map);

		this->map = map;
		this->width = map->width / PER_SECTOR;
		this->height = map->height / PER_SECTOR;

		fields.reserve(this->width * this->height);
		for (int y = 0; y < this->height; ++y) {
			for (int x = 0; x < this->width; ++x) {
				fields.push_back(std::vector<FlowField>());
				fields.back().reserve(PER_SECTOR * PER_SECTOR);
				this->update(x, y);
			}
		}

	}




};

class FlowFieldPathFinder {
	FlowFields *flowFields;
	std::vector<BorderPosition> borders;
public:
	void setFlowFields(FlowFields *f) {
		flowFields = f;
	}

	sf::Vector2i next(int cx, int cy, int dx, int dy) {
		int curpartx = cx / PER_SECTOR;
		int curparty = cy / PER_SECTOR;

		int partx = dx / PER_SECTOR;
		int party = dy / PER_SECTOR;
		int grx = dx % PER_SECTOR;
		int gry = dy % PER_SECTOR;

		sf::Vector2i npos;
		// on same part
		if (curpartx == partx && curparty == party) {
			int partIdx = partx + party * flowFields->width;
			int grIdx = grx + gry * PER_SECTOR;
			std::cout << "FlowFieldPathFinder at "<<partIdx<<" "<<grIdx<< " "<<flowFields->fields.size()<<std::endl;
			FlowField &flowField = flowFields->fields[partIdx][grIdx];

			npos = flowField.next(sf::Vector2i(cx, cy));
			std::cout << "FlowFieldPathFinder nexpos "<<npos.x<<"x"<<npos.y<<std::endl;
		}
		// TODO else
		return npos;
	}

	bool startFindPath(int sx, int sy, int dx, int dy) {
		borders.clear();
		std::cout << "FlowFieldPathFinder findPath " << sx << "x" << sy << " -> " << dx << "x" << dy << std::endl;
		JPS::PathVector path;
		bool found = flowFields->search->findPath(path, JPS::Pos(sx, sy), JPS::Pos(dx, dy), 1);
		if (found) {
			// find border pos
			for (auto &p : path) {
				int partx = p.x / PER_SECTOR;
				int party = p.y / PER_SECTOR;
				int partIdx = partx + party * flowFields->width;
				int grx = p.x % PER_SECTOR;
				int gry = p.y % PER_SECTOR;
				// border
				if (grx == 1 || gry == 1 || grx == PER_SECTOR - 1 || gry == PER_SECTOR - 1)
				{
					borders.push_back(BorderPosition{grx, gry, partIdx});
				}
			}

			for (auto &p : borders) {
				std::cout << "FlowFields should traverse " << p.partIdx << " -> " << p.x << "x" << p.y << std::endl;
			}

			return true;
		}
		return false;
	}

};
