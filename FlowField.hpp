#pragma once

#include "Config.hpp"
#include "Map.hpp"
#include "third_party/JPS.h"

#include "Steering.hpp"

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

	void setGrid(Map *m, sf::IntRect r) {
		map = m;
		rect = r;
		width = rect.width;
		height = rect.height;
	}

	bool localBound(int x, int y) const {
		return (x >= 0 && y >= 0 && x < rect.width && y < rect.height);
//		return true;
	}

	bool pathAvailable(int x, int y) const {
//		std::cout << "AVAILABLE "<<rect.left << "x" <<rect.top << " : "<<x+rect.left<<"x"<<y+rect.top<<std::endl;
		return this->localBound(x, y) && map->pathAvailable(x + rect.left, y + rect.top);
	}

	bool pathPrevision(int x, int y) const {
//		std::cout << "AVAILABLE "<<rect.left << "x" <<rect.top << " : "<<x+rect.left<<"x"<<y+rect.top<<std::endl;
		return this->localBound(x, y) && map->movingPathfinding.get(x + rect.left, y + rect.top) == 0;
	}

	bool pathUnit(int x, int y) const {
//		std::cout << "AVAILABLE "<<rect.left << "x" <<rect.top << " : "<<x+rect.left<<"x"<<y+rect.top<<std::endl;
		return this->localBound(x, y) && map->dynamicPathfinding.get(x + rect.left, y + rect.top) == 0;
	}

	bool bound(int x, int y) const {
		return this->localBound(x, y) && map->bound(x + rect.left, y + rect.top);
	}

};

class FlowField {
	std::vector<field_t> _fields;
	std::vector<dir_t> _dir;
	Grid _grid;

public:
	FlowField() {
	}

	void resize() {
		_fields.clear();
		_dir.clear();
		_fields.reserve(_grid.width * _grid.height);
		while (_fields.size() < _grid.width * _grid.height) {
			_fields.push_back(std::numeric_limits<field_t>::max());
		}
		_dir.reserve(_grid.width * _grid.height);
		while (_dir.size() < _grid.width * _grid.height) {
			_dir.push_back(-1);
		}
	}

	void setGrid(Grid grid) {
		_grid = grid;
	}

	void setGrid(Map *map, sf::IntRect rect) {
		_grid = Grid(map, rect);
	}

	void setGrid(Map *map) {
		_grid = Grid(map);
	}

	~FlowField() {
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
	int getNeighbors(int x, int y, int * ret) {
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
		/*
				if (_grid.bound(x, y - 1) && _grid.pathAvailable(x - 1, y - 1)) {
					ret[cnt++] = (x - 1) + (y - 1) * _grid.width;
				}
				if (_grid.bound(x, y + 1) && _grid.pathAvailable(x + 1, y + 1)) {
					ret[cnt++] = (x + 1) + (y + 1) * _grid.width;
				}
				if (_grid.bound(x - 1, y) && _grid.pathAvailable(x - 1, y + 1)) {
					ret[cnt++] = (x - 1) + (y + 1) * _grid.width;
				}
				if (_grid.bound(x + 1, y) && _grid.pathAvailable(x + 1, y - 1)) {
					ret[cnt++] = (x + 1) + (y - 1) * _grid.width;
				}
				*/
		return cnt;
	}

// -------------------------------------------------------------
// find the index of the neighbor with the lowest cost
// -------------------------------------------------------------
	int findLowestCost(int x, int y) {
		int m = std::numeric_limits<field_t>::max();
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
	}

// -------------------------------------------------------------
// build the flow field
// -------------------------------------------------------------
	void build(const sf::Vector2i & end) {
		// simple Dijstra flood fill first
		unsigned int targetID = end.y * _grid.width + end.x;
//		std::cout << "BUILD " << _grid.rect.left << "x" << _grid.rect.top << " " << _grid.width << "x" << _grid.height << " " << targetID << std::endl;
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
			int neighborCount = getNeighbors(currentX, currentY, neighbors);
//			std::cout << "GET neighbors "<<currentID << " "<<currentX<<"x"<<currentY<<" "<<neighborCount<<std::endl;
			for (int i = 0; i < neighborCount; ++i) {

#ifdef PATHFINDING_FLOWFIELD_DYNAMIC
				int modifier = 1;
				if (!_grid.pathPrevision(currentX, currentY))
					modifier = 256;
				/*				if (!_grid.pathUnit(currentX, currentY))
									modifier = 16;
				*/
				unsigned int endNodeCost = _fields[currentID] + modifier;
#else
				unsigned int endNodeCost = _fields[currentID] + 1;
#endif
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
//					std::cout << "FlowField path available " << x << "x" << y << std::endl;
					_dir[x + _grid.width * y] = findLowestCost(x, y);
				}
				else {
//					std::cout << "FlowField path not available "<<x<<"x"<<y<<std::endl;
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
	sf::Vector2i next(sf::Vector2i current) {
		int dir = get(current.x, current.y);
		if (!this->found(current)) {
			std::cout << "FlowField BUG invalid next dir:" << dir << " " << current.x << "x" << current.y << std::endl;
			return current;
		}
		return current + DIRECTIONS[dir];
	}

	bool found(sf::Vector2i current) {
		int dir = get(current.x, current.y);
		if (dir < 8)
			return true;
		else
			return false;
	}

	sf::Vector2i getSize() {
		return sf::Vector2i(_grid.width, _grid.height);
	}

};


#define PER_SECTOR 12

class FlowFieldPathFind {
	JPS::Searcher<Map> *search;
public:
	Map *map;

	void init(Map *map) {
		search = new JPS::Searcher<Map>(*map);
		this->map = map;
	}

	bool find(JPS::PathVector &path, int sx, int sy, int dx, int dy) {
		return search->findPath(path, JPS::Pos(sx, sy), JPS::Pos(dx, dy), 1);
	}
};

class FlowFieldPath {
	FlowFieldPathFind *pathFind;
	std::list<sf::Vector2i> traversed;
	std::list<sf::Vector2i> pathPoints;
	sf::Vector2i cur;
	sf::Vector2i dest;
	bool found;
	FlowField currentFlowField;
public:

	FlowField *getCurrentFlowField() {
		return &this->currentFlowField;
	}

	sf::Vector2i offset(sf::Vector2i p) const {
		return this->offset(p.x, p.y);
	}

	sf::Vector2i offset(int x, int y) const {
		return sf::Vector2i(x - PER_SECTOR / 2, y - PER_SECTOR / 2);
	}

	sf::Vector2i next(int cx, int cy, int dx, int dy) {
		sf::Vector2i cpos(cx, cy);
		if (cur != cpos) {
			traversed.push_back(cur);
			if (traversed.size() > 4)
				traversed.pop_front();
			cur = cpos;
		}

		sf::Vector2i npos(cx, cy);
		sf::Vector2i dpos(dx, dy);

		sf::Vector2i offset = this->offset(cpos);
		sf::Vector2i cgpos = cpos - offset;

		sf::IntRect ffRect = sf::IntRect(offset.x, offset.y, PER_SECTOR, PER_SECTOR);

		this->currentFlowField.setGrid(pathFind->map, ffRect);

		sf::Vector2i ndpos(dx, dy);

		if (ffRect.contains(dpos)) {
#ifdef FLOWFIELDS_DEBUG
			std::cout << "FlowFieldPath dest in sector " << dpos.x << "x" << dpos.y << std::endl;
#endif
		} else {
#ifdef FLOWFIELDS_DEBUG
			std::cout << "FlowFieldPath dest out of sector " << dpos.x << "x" << dpos.y << " (" << pathPoints.size() << ")" << std::endl;
#endif
			std::vector<sf::Vector2i> possibleDest;
			for (sf::Vector2i np : pathPoints) {
				if (np.x >= offset.x && np.x < offset.x + PER_SECTOR && np.y >= offset.y && np.y < offset.y + PER_SECTOR && np != cpos) {
#ifdef FLOWFIELDS_DEBUG
					std::cout << "FlowFieldPath add possible dest " << np.x << "x" << np.y << std::endl;
#endif
					possibleDest.push_back(np);
				}
			}
			float distance = std::numeric_limits<float>::max();
			for (sf::Vector2i np : possibleDest) {
				if (vectorLength(dpos - np) < distance) {
#ifdef FLOWFIELDS_DEBUG
					std::cout << "FlowFieldPath best found " << " " << vectorLength(dpos - np) << " < " << distance << std::endl;
#endif
					distance = vectorLength(dpos - np);
					ndpos = np;
				}
			}
#ifdef FLOWFIELDS_DEBUG
			std::cout << "FlowFieldPath best local dest " << ndpos.x << "x" << ndpos.y << std::endl;
#endif
		}

		ndpos -= offset;
		this->currentFlowField.build(ndpos);

		if (this->currentFlowField.found(cgpos)) {
			npos = this->currentFlowField.next(cgpos);
			npos += offset;
		} else {
			// follow JPS path if no flow field found
			ndpos = cpos;
			for (sf::Vector2i np : pathPoints) {
				float distance = std::numeric_limits<float>::max();
				if (np.x >= offset.x && np.x < offset.x + PER_SECTOR && np.y >= offset.y && np.y < offset.y + PER_SECTOR && np != cpos) {
					if (vectorLength(np - cpos) < distance) {
						distance = vectorLength(np - cpos);
						ndpos = np;
//						std::cout << "FlowFieldPath fallback "<<ndpos.x<<"x"<<ndpos.y<<std::endl;
					}
				}
			}

			ndpos -= offset;
			this->currentFlowField.build(ndpos);

			if (this->currentFlowField.found(cgpos)) {
				npos = this->currentFlowField.next(cgpos);
				npos += offset;
			} else {
//				std::cout << "FlowFieldPath really cannot found next pos " << cpos.x << "x" << cpos.y << " " << dpos.x << "x" << dpos.y << std::endl;
			}


			/*
						 for (std::list<sf::Vector2i>::iterator it=pathPoints.begin(); it != pathPoints.end(); ++it) {
						 	if(*it == cpos) {
						 		std::list<sf::Vector2i>::iterator nposIt = std::next(it);
						 		if(nposIt != pathPoints.end())
						 			npos = *nposIt;
						 	}
						 }
						 */
		}

#ifdef FLOWFIELDS_DEBUG
		std::cout << "FlowFieldPath nextpos " << cx << "x" << cy << " -> " << npos.x << "x" << npos.y << " " << offset.x << "x" << offset.y << std::endl;
#endif
		return npos;
	}

	bool start(int sx, int sy, int dx, int dy) {
		if (sf::Vector2i(dx, dy) != dest) {
			traversed.clear();
			pathPoints.clear();
			dest = sf::Vector2i(dx, dy);

			JPS::PathVector path;
			this->found = pathFind->find(path, sx, sy, dx, dy);

			if (this->found) {
				// find border pos
				for (auto &p : path) {
					this->pathPoints.push_back(sf::Vector2i(int(p.x), int(p.y)));
				}
			}

		} else {
			// check if destination position is still available
			if (this->found)
				this->found = this->pathFind->map->pathAvailable(dest.x, dest.y);
		}
		return this->found;
	}

	void setPathFind(FlowFieldPathFind *p) {
		pathFind = p;
	}
};