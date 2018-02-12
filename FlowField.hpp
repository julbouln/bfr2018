#pragma once

#include "Map.hpp"

typedef short int field_t;
typedef signed char dir_t;

const sf::Vector2i DIRECTIONS[] = { sf::Vector2i(1, 0), sf::Vector2i(1, 1), sf::Vector2i(0, 1), sf::Vector2i(-1, 1),
                                    sf::Vector2i(-1, 0), sf::Vector2i(-1, -1), sf::Vector2i(0, -1), sf::Vector2i(1, -1)
                                  };

class FlowField {
	field_t* _fields;
	dir_t* _dir;
	Map* _grid;

public:
	FlowField() {
		_fields = nullptr;
		_dir = nullptr;
	}

	void setGrid(Map *grid) {
		_grid = grid;
		if (!_fields)
			_fields = new field_t[_grid->width * _grid->height];
		if (!_dir)
			_dir = new dir_t[_grid->width * _grid->height];
	}

	FlowField(Map* grid) : _grid(grid) {
		_fields = new field_t[_grid->width * _grid->height];
		_dir = new dir_t[_grid->width * _grid->height];
	}

	~FlowField() {
		if (_dir)
			delete[] _dir;
		if (_fields)
			delete[] _fields;
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
		if (_grid->bound(x, y - 1) && _grid->pathAvailable(x, y - 1)) {
			ret[cnt++] = x + (y - 1) * _grid->width;
		}
		if (_grid->bound(x, y + 1) && _grid->pathAvailable(x, y + 1)) {
			ret[cnt++] = x + (y + 1) * _grid->width;
		}
		if (_grid->bound(x - 1, y) && _grid->pathAvailable(x - 1, y)) {
			ret[cnt++] = x - 1 + y * _grid->width;
		}
		if (_grid->bound(x + 1, y) && _grid->pathAvailable(x + 1, y)) {
			ret[cnt++] = x + 1 + y * _grid->width;
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
			if (_grid->pathAvailable(c.x, c.y)) {
				int idx = c.x + c.y * _grid->width;
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
		int total = _grid->width * _grid->height;
		for (int i = 0; i < total; ++i) {
			_fields[i] = std::numeric_limits<short int>::max();
			_dir[i] = -1;
		}
	}

// -------------------------------------------------------------
// build the flow field
// -------------------------------------------------------------
	void build(const sf::Vector2i & end) {
		// simple Dijstra flood fill first
		unsigned int targetID = end.y * _grid->width + end.x;
		resetFields();
		std::list<unsigned int> openList;
		_fields[targetID] = 0;
		openList.push_back(targetID);
		int neighbors[4];
		while (openList.size() > 0)	{
			unsigned currentID = openList.front();
			openList.pop_front();
			unsigned short currentX = currentID % _grid->width;
			unsigned short currentY = currentID / _grid->width;
			int neighborCount = getNeighbors(currentX, currentY, neighbors, 4);
			for (int i = 0; i < neighborCount; ++i) {
				unsigned int endNodeCost = _fields[currentID] + 1;
				if (endNodeCost < _fields[neighbors[i]]) {
					if (!checkIfContains(neighbors[i], openList)) {
						openList.push_back(neighbors[i]);
					}
					_fields[neighbors[i]] = endNodeCost;
				}
			}

		}
		// now calculate the directions
		for (int x = 0; x < _grid->width; ++x) {
			for (int y = 0; y < _grid->height; ++y) {
				if (_grid->pathAvailable(x, y)) {
					_dir[x + _grid->width * y] = findLowestCost(x, y);
				}
				else {
					_dir[x + _grid->width * y] = 16;
				}
			}
		}
		std::cout << "FlowField: built " << end.x << "x" << end.y << std::endl;
	}

// -------------------------------------------------------------
// get direction
// -------------------------------------------------------------
	int get(int x, int y) const {
		int idx = x + y * _grid->width;
		std::cout << "FlowField: get " << x << "x" << y << " : " << idx << " " << _dir[idx] << std::endl;
		return _dir[idx];
	}

// -------------------------------------------------------------
// get cost of cell
// -------------------------------------------------------------
	int getCost(int x, int y) const {
		int idx = x + y * _grid->width;
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