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
//		_fields = nullptr;
//		_dir = nullptr;
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

	}

	void setGrid(Map *map, sf::IntRect rect) {
//		std::cout << "SET GRID " << rect.left << "x" << rect.top << ":" << rect.width << "x" << rect.height << std::endl;
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
		std::cout << "FlowField next dir:" << dir << " " << current.x << "x" << current.y << std::endl;
		return current + DIRECTIONS[dir];
	}

	sf::Vector2i getSize() {
		return sf::Vector2i(_grid.width,_grid.height);
	}

};


#define PER_SECTOR 12

struct SectorPathPosition {
	int x;
	int y;
	int partIdx;
	int border;
};

enum Border {
	None,
	Left,
	Top,
	Right,
	Bottom
};

struct BorderTransition {
	int partIdx;
	int border;
	sf::Vector2i dir;
	sf::Vector2i sPos;
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
		for (int dy = 0; dy < PER_SECTOR; ++dy) {
			for (int dx = 0; dx < PER_SECTOR; ++dx) {
//						std::cout << "BUILD FIELD " << x << "x" << y << " : " << gx << "x" << gy << " "<<map->pathAvailable(x*PER_SECTOR+gx,y*PER_SECTOR+gy) << std::endl;
//				std::cout << "SET GRID " << x * PER_SECTOR << "x" << y * PER_SECTOR << std::endl;
				fields[idx].push_back(FlowField());
				FlowField &field = fields[idx].back();

				field.setGrid(map, sf::IntRect(x * PER_SECTOR, y * PER_SECTOR, PER_SECTOR, PER_SECTOR));
				field.build(sf::Vector2i(dx, dy));
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
			}
		}
		for (int y = 0; y < this->height; ++y) {
			for (int x = 0; x < this->width; ++x) {
				this->update(x, y);
			}
		}
	}
};



class FlowFieldPathFinder {
	FlowFields *flowFields;
	std::vector<SectorPathPosition> sectorPaths;
	std::vector<BorderTransition> transitions;
	sf::Vector2i dest;
	bool found;
public:
	void setFlowFields(FlowFields *f) {
		flowFields = f;
	}

	int sectorIdx(int x, int y) {
		int px = x / PER_SECTOR;
		int py = y / PER_SECTOR;
		return px + py * flowFields->width;
	}

	int gridIdx(int x, int y) {
		int gx = x % PER_SECTOR;
		int gy = y % PER_SECTOR;
		return gx + gy * PER_SECTOR;
	}

	sf::Vector2i gridPos(int x, int y) {
		return sf::Vector2i(x % PER_SECTOR, y % PER_SECTOR);
	}

	sf::Vector2i sectorPos(int x, int y) {
		return sf::Vector2i(x / PER_SECTOR, y / PER_SECTOR);
	}

	FlowField &getFlowField(int x, int y) {
		return flowFields->fields[this->sectorIdx(x, y)][this->gridIdx(x, y)];
	}

	FlowField &getCurrentFlowField(int cx, int cy, int dx, int dy) {
		int curPartIdx = this->sectorIdx(cx, cy);
		int partIdx = this->sectorIdx(dx, dy);
		int grIdx = this->gridIdx(dx, dy);

		if (partIdx == curPartIdx) {
			std::vector<FlowField> &sectorFields = flowFields->fields[partIdx];
//			std::cout << "FlowFieldPathFinder at " << partIdx << "/" << flowFields->fields.size() << " " << grIdx << "/" << sectorFields.size() << std::endl;
			FlowField &flowField = sectorFields[grIdx];

			return flowField;
		} else {
			for (BorderTransition trans : transitions) {
				if (trans.partIdx == curPartIdx) {
					int lIdx = this->gridIdx(trans.sPos.x,trans.sPos.y);
					//std::cout << "FlowFieldPathFinder traverse dest " << p.x << "x" << p.y << " " << lx << "x" << ly << std::endl;
					FlowField &flowField = flowFields->fields[curPartIdx][lIdx];
					return flowField;
				}

			}
			std::cout << "BUG: cannot found flowfield" << std::endl;;
		}
	}

	sf::Vector2i next(int cx, int cy, int dx, int dy) {
		sf::Vector2i cpos = this->gridPos(cx, cy);

		int curPartIdx = this->sectorIdx(cx, cy);
		int partIdx = this->sectorIdx(dx, dy);
		int grIdx = this->gridIdx(dx, dy);

		std::cout << "FlowFieldPathFinder next " << cx << "x" << cy << " " << dx << "x" << dy << std::endl;

		sf::Vector2i npos;
		// on same part
		FlowField &flowField = this->getCurrentFlowField(cx,cy,dx,dy);
		npos = flowField.next(cpos);

		if (partIdx != curPartIdx) {
			int border = this->onSectorBorder(cpos.x, cpos.y);
			if (border) {
				for (BorderTransition trans : transitions) {
//					std::cout << "FlowFieldPathFinder check border "<<trans.partIdx << " "<<trans.border << " : "<<curPartIdx << " "<<border << std::endl;
					if (trans.partIdx == curPartIdx && trans.border == border) {
						std::cout << "FlowFieldPathFinder ON BORDER " << trans.dir.x << "x" << trans.dir.y << std::endl;;
						npos.x = cpos.x + trans.dir.x;
						npos.y = cpos.y + trans.dir.y;
					}
				}
			}
//			npos = flowField.next(sf::Vector2i(cgrx, cgry));
			std::cout << "FlowFieldPathFinder need to pass through " << partIdx << std::endl;

		}


		sf::Vector2i spos = this->sectorPos(cx, cy) * PER_SECTOR;
		npos += spos;
		std::cout << "FlowFieldPathFinder nextpos " << npos.x << "x" << npos.y << std::endl;

		return npos;
	}

	int onSectorBorder(int x, int y) {
//		return (x == 0 || y == 0 || x == PER_SECTOR - 1 || y == PER_SECTOR - 1);
		if (x == 0)
			return Left;
		if (y == 0)
			return Top;
		if (x == PER_SECTOR - 1)
			return Right;
		if (y == PER_SECTOR - 1)
			return Bottom;

		return None;
	}

	bool startFindPath(int sx, int sy, int dx, int dy) {
		if (sf::Vector2i(dx, dy) != dest) {
			dest = sf::Vector2i(dx, dy);
			sectorPaths.clear();
			transitions.clear();
			std::cout << "FlowFieldPathFinder findPath " << sx << "x" << sy << " -> " << dx << "x" << dy << std::endl;
			JPS::PathVector path;
			this->found = flowFields->search->findPath(path, JPS::Pos(sx, sy), JPS::Pos(dx, dy), 1);
			if (this->found) {
				// find border pos
				for (auto &p : path) {
					int partx = p.x / PER_SECTOR;
					int party = p.y / PER_SECTOR;
					int partIdx = partx + party * flowFields->width;
					int grx = p.x % PER_SECTOR;
					int gry = p.y % PER_SECTOR;
					// border
					int border = this->onSectorBorder(grx, gry);
					if (border)
					{
						if (sectorPaths.size() > 0) {
							SectorPathPosition prevPos = sectorPaths.back();
							if (prevPos.partIdx != partIdx) {
								sf::Vector2i dir(p.x - prevPos.x, p.y - prevPos.y);
								std::cout << "FlowFieldPathFinder prev " << prevPos.partIdx << " -> " << partIdx << " " << prevPos.x << "x" << prevPos.y << " " << p.x << "x" << p.y << " " << dir.x << "x" << dir.y << std::endl;
								transitions.push_back(BorderTransition{prevPos.partIdx, prevPos.border, dir, sf::Vector2i(prevPos.x,prevPos.y)});
							}
						}

						sectorPaths.push_back(SectorPathPosition{(int)p.x, (int)p.y, partIdx, border});
					}
				}

				for (auto &p : sectorPaths) {
					std::cout << "FlowFields should traverse " << p.partIdx << " -> " << p.x << "x" << p.y << std::endl;
				}
			}
			else {
				std::cout << "FlowFieldPathFinder path not found "<<sx<<"x"<<sy<<" -> "<<dx<<"x"<<dy<<std::endl;
			}

		}
		return this->found;
	}

};
