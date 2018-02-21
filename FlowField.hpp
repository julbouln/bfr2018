#pragma once

#include "Config.hpp"
#include "Map.hpp"
#include "third_party/JPS.h"

typedef short int field_t;
typedef signed char dir_t;

const sf::Vector2i DIRECTIONS[] = { sf::Vector2i(1, 0), sf::Vector2i(1, 1), sf::Vector2i(0, 1), sf::Vector2i(-1, 1),
                                    sf::Vector2i(-1, 0), sf::Vector2i(-1, -1), sf::Vector2i(0, -1), sf::Vector2i(1, -1)
                                  };

struct MovingObject {
	sf::Vector2f pos;
	sf::Vector2f velocity;
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
		return this->localBound(x, y) && map->pathAvailable(x + rect.left, y + rect.top) && map->movingPathfinding.get(x + rect.left, y + rect.top) == 0;
	}

	bool pathPrevision(int x, int y) const {
//		std::cout << "AVAILABLE "<<rect.left << "x" <<rect.top << " : "<<x+rect.left<<"x"<<y+rect.top<<std::endl;
		return this->localBound(x, y) && map->movingPathfinding.get(x + rect.left, y + rect.top) == 0;
		/*		for(int cx = x-1;cx < x+2;cx++) {
					for(int cy = y-1;cy < y+2;cy++) {
						if(this->map->bound(cx,cy)) {
							EntityID ent = this->map.objs.get(cx,cy);
						}
					}
				}
				*/
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
//				if (!_grid.pathPrevision(currentX, currentY)) {
//					std::cout << "WARN already at "<<currentX<<"x"<<currentY<<std::endl;
//					modifier = 16;
//				}
				/*				if (!_grid.pathUnit(currentX, currentY))
									modifier = 16;
				*/
				unsigned int endNodeCost = _fields[currentID] + modifier;
#else
				unsigned int endNodeCost = _fields[currentID] + 1;
#endif
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
				if (_grid.pathAvailable(x, y)) {
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
		if (!search)
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

enum class FlowFieldMode {
	Pathfinding,
	Steering
};

class FlowFieldPath {
	FlowFieldPathFind *pathFind;
	std::list<sf::Vector2i> traversed;
	std::list<sf::Vector2i> pathPoints;
	sf::Vector2i cur;
	sf::Vector2i dest;
	FlowFieldMode mode;
	bool found;
	FlowField currentFlowField;
public:
	sf::Vector2i ffDest;

	FlowFieldPath() {
		this->mode = FlowFieldMode::Pathfinding;
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
		sf::Vector2f seekVel = vectorNormalize(sf::Vector2f(dpos - cpos));
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

	sf::Vector2i nearestPathPoint(sf::Vector2i cpos) {
		sf::Vector2i offset = this->offset(cpos);
		sf::Vector2i point = cpos;
		float distance = std::numeric_limits<float>::max();
		for (sf::Vector2i np : this->pathPoints) {
			if (np.x >= offset.x && np.x < offset.x + PER_SECTOR &&
			        np.y >= offset.y && np.y < offset.y + PER_SECTOR &&
			        np != cpos && this->pathFind->map->pathAvailable(np.x,np.y)) {
				if (vectorLength(np - cpos) < distance) {
					distance = vectorLength(np - cpos);
					point = np;
//						std::cout << "FlowFieldPath fallback "<<ndpos.x<<"x"<<ndpos.y<<std::endl;
				}
			}
		}
		return point;
	}

	sf::Vector2i farestPathPoint(sf::Vector2i cpos) {
		sf::Vector2i offset = this->offset(cpos);
		sf::Vector2i point = cpos;
		float distance = std::numeric_limits<float>::max();
		for (sf::Vector2i np : this->pathPoints) {
			if (np.x >= offset.x && np.x < offset.x + PER_SECTOR &&
			        np.y >= offset.y && np.y < offset.y + PER_SECTOR &&
			        np != cpos) {
				if (vectorLength(dest - np) < distance) {
					distance = vectorLength(dest - np);
					point = np;
				}
			}
		}
		return point;
	}

	sf::Vector2i bestFollowingPathPoint(sf::Vector2i cpos) {
		sf::Vector2i offset = this->offset(cpos);
		sf::Vector2i point = cpos;
		int bcnt = 0;
		int pcnt = 0;
		for (sf::Vector2i np : this->pathPoints) {
			if (np.x >= offset.x && np.x < offset.x + PER_SECTOR &&
			        np.y >= offset.y && np.y < offset.y + PER_SECTOR &&
			        np != cpos) {
				if (pcnt > bcnt) {
					bcnt = pcnt;
					point = np;
				}
			}
			pcnt++;
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
						if (x >= offset.x && x < offset.x + PER_SECTOR && y >= offset.y && y < offset.y + PER_SECTOR && this->pathFind->map->bound(x, y) && this->pathFind->map->pathAvailable(x, y)) {
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

		if (this->mode == FlowFieldMode::Pathfinding) {

			sf::Vector2i offset = this->offset(cpos);
			sf::Vector2i cgpos = cpos - offset;

			sf::IntRect ffRect = sf::IntRect(offset.x, offset.y, PER_SECTOR, PER_SECTOR);

			this->currentFlowField.setGrid(pathFind->map, ffRect);

			sf::Vector2i ndpos = dest;

			if (ffRect.contains(dest)) {
#ifdef FLOWFIELDS_DEBUG
				std::cout << "FlowFieldPath dest in sector " << dest.x << "x" << dest.y << std::endl;
#endif
			} else {
#ifdef FLOWFIELDS_DEBUG
				std::cout << "FlowFieldPath dest out of sector " << dest.x << "x" << dest.y << " (" << pathPoints.size() << ")" << std::endl;
#endif
//			ndpos = this->farestPathPoint(cpos);
				ndpos = this->bestFollowingPathPoint(cpos);
				if(!this->pathFind->map->pathAvailable(ndpos.x,ndpos.y)) {
					ndpos = this->firstFreePos(cpos,ndpos,1,2);
				}
#ifdef FLOWFIELDS_DEBUG
				std::cout << "FlowFieldPath best local dest " << ndpos.x << "x" << ndpos.y << std::endl;
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
						std::cout << "FlowFieldPath cannot found next pos " << cpos.x << "x" << cpos.y << " " << dest.x << "x" << dest.y << " " << npos.x << "x" << npos.y << std::endl;
#endif

#if 0				
				// follow JPS path if no flow field found
				bool foundNearPath = false;
				for (std::list<sf::Vector2i>::iterator it = pathPoints.begin(); it != pathPoints.end(); ++it) {
					if (*it == cpos) {
						std::list<sf::Vector2i>::iterator nposIt = std::next(it);
						if (nposIt != pathPoints.end()) {
							npos = *nposIt;
							foundNearPath = true;
							break;
						}
					}
				}

				if (!foundNearPath) {
					sf::Vector2i nearPathPoint = this->nearestPathPoint(cpos);
					ndpos = nearPathPoint;
					ndpos -= offset;
					this->currentFlowField.build(ndpos);

					if (this->currentFlowField.found(cgpos)) {
						npos = this->currentFlowField.next(cgpos);
						npos += offset;
					} else {
//						sf::Vector2i steer = sf::Vector2i(vectorRound(this->seek(cpos, nearPathPoint)));
//						npos = cpos + steer;
					npos = cpos;
#ifdef FLOWFIELDS_DEBUG
						std::cout << "FlowFieldPath really cannot found next pos " << cpos.x << "x" << cpos.y << " " << dest.x << "x" << dest.y << " " << npos.x << "x" << npos.y << std::endl;
					}
#endif
				}
#endif
			}

#ifdef FLOWFIELDS_DEBUG
			std::cout << "FlowFieldPath nextpos " << cx << "x" << cy << " -> " << npos.x << "x" << npos.y << " " << offset.x << "x" << offset.y << std::endl;
#endif
		} else {
			sf::Vector2i steer = sf::Vector2i(vectorRound(this->seek(cpos, dest)));
			npos = cpos + steer;
		}

		return npos;
	}

	bool start(int sx, int sy, int dx, int dy) {
		if (sf::Vector2i(dx, dy) != dest) {
			traversed.clear();
			pathPoints.clear();
			dest = sf::Vector2i(dx, dy);
			cur = sf::Vector2i(sx, sy);

			if (vectorLength(dest - cur) < 1.5) {
				this->mode = FlowFieldMode::Steering;
				this->ffDest = dest;
				this->found = this->pathFind->map->pathAvailable(dest.x, dest.y);
			} else {
				this->mode = FlowFieldMode::Pathfinding;
				JPS::PathVector path;
				this->found = pathFind->find(path, sx, sy, dx, dy);

				if (this->found) {
					// find border pos
					for (auto &p : path) {
						this->pathPoints.push_back(sf::Vector2i(int(p.x), int(p.y)));
					}
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