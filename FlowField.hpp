#pragma once

#include <iostream>

#include "Config.hpp"
#include "Map.hpp"
#include "third_party/JPS.h"

#include "Components/Components.hpp"

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
	}


	bool pathAvailable(int x, int y) const {
//		std::cout << "AVAILABLE "<<rect.left << "x" <<rect.top << " : "<<x+rect.left<<"x"<<y+rect.top<<std::endl;
		return localBound(x, y) && map->pathAvailable(x + rect.left, y + rect.top);
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

	Grid *getGrid() {
		return &_grid;
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

	bool pathAvailable(int x, int y) {
		return _grid.pathAvailable(x, y);
	}

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

// get neighbors (N, S, W and E). Will return the indices
	int getNeighbors(int x, int y, int * ret, bool *moving) {
		int cnt = 0;
		if (_grid.bound(x, y - 1) && this->pathAvailable(x, y - 1)) {
			ret[cnt++] = x + (y - 1) * _grid.width;
		}
		if (_grid.bound(x, y + 1) && this->pathAvailable(x, y + 1)) {
			ret[cnt++] = x + (y + 1) * _grid.width;
		}
		if (_grid.bound(x - 1, y) && this->pathAvailable(x - 1, y)) {
			ret[cnt++] = x - 1 + y * _grid.width;
		}
		if (_grid.bound(x + 1, y) && this->pathAvailable(x + 1, y)) {
			ret[cnt++] = x + 1 + y * _grid.width;
		}
		return cnt;
	}

	int findLowestCost(int x, int y) {
		int m = std::numeric_limits<field_t>::max();
		int ret = 14;
		for (int i = 0; i < 8; ++i) {
			sf::Vector2i c = sf::Vector2i(x, y) + DIRECTIONS[i];
			if (this->pathAvailable(c.x, c.y)) {
				int idx = c.x + c.y * _grid.width;
				if (_fields[idx] < m) {
					ret = i;
					m = _fields[idx];
				}
			}
		}
		return ret;
	}

	void resetFields() {
		this->resize();
	}

	void build(const sf::Vector2i & end) {
		// simple Dijstra flood fill first
		unsigned int targetID = end.y * _grid.width + end.x;
//		std::cout << "BUILD " << _grid.rect.left << "x" << _grid.rect.top << " " << _grid.width << "x" << _grid.height << " " << targetID << std::endl;
		resetFields();
		std::list<unsigned int> openList;
		_fields[targetID] = 0;
		openList.push_back(targetID);
		int neighbors[4];
		bool movNeighbors[4];
		while (openList.size() > 0)	{
			unsigned currentID = openList.front();
			openList.pop_front();
			unsigned short currentX = currentID % _grid.width;
			unsigned short currentY = currentID / _grid.width;
			int neighborCount = getNeighbors(currentX, currentY, neighbors, movNeighbors);
//			std::cout << "GET neighbors "<<currentID << " "<<currentX<<"x"<<currentY<<" "<<neighborCount<<std::endl;
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
		for (int x = 0; x < _grid.width; ++x) {
			for (int y = 0; y < _grid.height; ++y) {
				if (this->pathAvailable(x, y)) {
					_dir[x + _grid.width * y] = findLowestCost(x, y);
				}
				else {
					_dir[x + _grid.width * y] = 16;
				}
			}
		}
	}

	int get(int x, int y) const {
		int idx = x + y * _grid.width;
		return _dir[idx];
	}

	int getCost(int x, int y) const {
		int idx = x + y * _grid.width;
		return _fields[idx];
	}

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

	~FlowFieldPathFind() {
		if (search)
			delete search;
	}

	FlowFieldPathFind() {
		search = nullptr;
	}

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

	int lastCalcDest;
public:
	sf::Vector2i ffDest;

	FlowFieldPath() {
		this->lastCalcDest = 4;
		this->cur = sf::Vector2i(-1, -1);
		this->dest = sf::Vector2i(-1, -1);
	}

	FlowField *getCurrentFlowField() {
		return &this->currentFlowField;
	}

	sf::Vector2i offset(sf::Vector2i p) const {
		return this->offset(p.x, p.y);
	}

	sf::Vector2i offset(int x, int y) const {
		return sf::Vector2i(x - PER_SECTOR / 2, y - PER_SECTOR / 2);
	}

	sf::Vector2f seek(sf::Vector2i cpos, sf::Vector2i dpos) {
		sf::Vector2f seekVel = normalize(sf::Vector2f(dpos - cpos));
//		std::cout << "FlowFieldPath: seek " << seekVel.x << "x" << seekVel.y << std::endl;
		return seekVel;
	}

	std::vector<sf::Vector2i> inRangePathPoints(sf::Vector2i cpos) {
		sf::Vector2i offset = this->offset(cpos);
		std::vector<sf::Vector2i> points;
		for (sf::Vector2i np : this->pathPoints) {
			if (np.x >= offset.x && np.x < offset.x + PER_SECTOR &&
			        np.y >= offset.y && np.y < offset.y + PER_SECTOR &&
			        np != cpos) {
				points.push_back(np);
			}
		}
		return points;
	}


	sf::Vector2i bestFollowingPathPoint(sf::Vector2i cpos) {
		sf::Vector2i offset = this->offset(cpos);
		sf::Vector2i point = cpos;
		int bcnt = 0;
		int pcnt = 0;
		for (sf::Vector2i np : this->pathPoints) {
			if (np.x >= offset.x && np.x < offset.x + PER_SECTOR &&
			        np.y >= offset.y && np.y < offset.y + PER_SECTOR &&
			        np != cpos) {// && this->pathFind->map->pathAvailable(np.x, np.y)) {
				if (pcnt > bcnt) {
					bcnt = pcnt;
					point = np;
				}
			}
			pcnt++;
		}

		sf::Vector2i bPoint = point;

		std::vector<sf::Vector2i> linePoints;

		if (point.x == offset.x) {
			// left
			for (int y = offset.y; y < offset.y + PER_SECTOR; y++) {
				if (this->pathFind->map->pathAvailable(point.x, y))
					linePoints.push_back(sf::Vector2i(point.x, y));
			}
		} else if (point.x == offset.x + PER_SECTOR - 1) {
			// right
			for (int y = offset.y; y < offset.y + PER_SECTOR; y++) {
				if (this->pathFind->map->pathAvailable(point.x, y))
					linePoints.push_back(sf::Vector2i(point.x, y));
			}
		}

		if (point.y == offset.y) {
			// top
			for (int x = offset.x; x < offset.x + PER_SECTOR; x++) {
				if (this->pathFind->map->pathAvailable(x, point.y))
					linePoints.push_back(sf::Vector2i(x, point.y));
			}
		} else if (point.y == offset.y + PER_SECTOR - 1) {
			// bottom
			for (int x = offset.x; x < offset.x + PER_SECTOR; x++) {
				if (this->pathFind->map->pathAvailable(x, point.y))
					linePoints.push_back(sf::Vector2i(x, point.y));
			}
		}

		if (linePoints.size() > 0) {
			float distance = std::numeric_limits<float>::max();
			for (sf::Vector2i np : linePoints) {
				if (length(np - cpos) < distance) {
					distance = length(np - cpos);
					point = np;
//						std::cout << "FlowFieldPath fallback "<<ndpos.x<<"x"<<ndpos.y<<std::endl;
				}
			}
		}

		return point;
	}

	sf::Vector2i firstFreePos(sf::Vector2i src, sf::Vector2i dest, int minDist, int maxDist) {
		sf::Vector2i offset = this->offset(src);
		int dist = minDist;
		while (dist < maxDist) {
			for (int w = -dist; w < dist + 1; ++w) {
				for (int h = -dist; h < dist + 1; ++h) {
					if (w == -dist || h == -dist || w == dist || h == dist) {
						int x = w + dest.x;
						int y = h + dest.y;
						if (x >= offset.x && x < offset.x + PER_SECTOR && y >= offset.y && y < offset.y + PER_SECTOR &&
						        this->pathFind->map->bound(x, y) &&
						        this->pathFind->map->pathAvailable(x, y)) {
							return sf::Vector2i(x, y);
						}
					}
				}
			}
			dist++;
		}
		return dest;
	}

	sf::Vector2i next(int cx, int cy) {
		sf::Vector2i cpos(cx, cy);
		if (cur != cpos) {
			traversed.push_back(cur);
			if (traversed.size() > 4)
				traversed.pop_front();
			cur = cpos;
		}

		sf::Vector2i npos(cx, cy);


		sf::Vector2i offset = this->offset(cpos);
		sf::Vector2i cgpos = cpos - offset;


		sf::Vector2i ndpos = dest;

		sf::IntRect ffRect = sf::IntRect(offset.x, offset.y, PER_SECTOR, PER_SECTOR);
//			sf::IntRect ffRect = rect(cpos, ndpos);

		this->currentFlowField.setGrid(pathFind->map, ffRect);


		if (ffRect.contains(dest)) {
			ndpos = dest;
#ifdef FLOWFIELDS_DEBUG
			std::cout << "FlowFieldPath dest in sector " << dest << std::endl;
#endif
		} else {
			ndpos = this->bestFollowingPathPoint(cpos);

#ifdef FLOWFIELDS_DEBUG
			std::cout << "FlowFieldPath dest out of sector " << dest << " (" << pathPoints.size() << ")" << std::endl;
#endif
			if (lastCalcDest < 4 && ffRect.contains(ffDest)) {
				ndpos = ffDest;
			} else {
				lastCalcDest = 0;
			}


			lastCalcDest++;
#ifdef FLOWFIELDS_DEBUG
			std::cout << "FlowFieldPath best local dest " << ndpos << std::endl;
#endif
		}

		ffDest = ndpos;
		ndpos -= offset;
		this->currentFlowField.build(ndpos);

		if (this->currentFlowField.found(cgpos)) {
			npos = this->currentFlowField.next(cgpos);
			npos += offset;
		} else {
			npos = cpos;

#ifdef FLOWFIELDS_DEBUG
			std::cout << "FlowFieldPath cannot found next pos " << cpos << " " << dest << " " << npos << std::endl;
#endif
		}

#ifdef FLOWFIELDS_DEBUG
		std::cout << "FlowFieldPath nextpos " << cpos << " -> " << npos << " " << offset << std::endl;
#endif


		return npos;
	}

	bool start(int sx, int sy, int dx, int dy) {
		sf::Vector2i newDest = sf::Vector2i(dx, dy);
		if (newDest != dest) {
			sf::Vector2i oldDest = dest;

			dest = newDest;
			cur = sf::Vector2i(sx, sy);
			traversed.clear();

			this->lastCalcDest = 4;

			this->ffDest = dest;

			JPS::PathVector path;
			this->found = pathFind->find(path, sx, sy, dx, dy);

			if (this->found) {
				// find border pos
				this->initPath(path);
			}
		} else {
			// check if destination position is still available
			if (this->found)
				this->found = this->pathFind->map->pathAvailable(dest.x, dest.y);
		}
		return this->found;
	}

	void initPath(JPS::PathVector &path) {
		pathPoints.clear();
		for (auto &p : path) {
			this->pathPoints.push_back(sf::Vector2i(int(p.x), int(p.y)));
		}
	}

	bool pathFound() {
		return this->found;
	}

	void setPathFind(FlowFieldPathFind *p) {
		pathFind = p;
		this->currentFlowField.setGrid(pathFind->map, sf::IntRect(0, 0, 0, 0));
	}

};