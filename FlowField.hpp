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
		return this->localBound(x, y) && map->movingPathfinding.get(x + rect.left, y + rect.top)==0;
	}

	bool pathUnit(int x, int y) const {
//		std::cout << "AVAILABLE "<<rect.left << "x" <<rect.top << " : "<<x+rect.left<<"x"<<y+rect.top<<std::endl;
		return this->localBound(x, y) && map->dynamicPathfinding.get(x + rect.left, y + rect.top)==0;
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

#ifdef PATHFINDING_FLOWFIELD_DYNAMIC
				int modifier = 1;
				if(!_grid.pathPrevision(currentX,currentY))
					modifier = 8;
				if(!_grid.pathUnit(currentX,currentY))
					modifier = 16;

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
//		std::cout << "FlowField next dir:" << dir << " " << current.x << "x" << current.y << std::endl;
		return current + DIRECTIONS[dir];
	}

	sf::Vector2i getSize() {
		return sf::Vector2i(_grid.width, _grid.height);
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
	std::set<int> markFlowFieldsUpdate;

	sf::Clock clock;
	Steering steering;
public:
	Map *map;
	JPS::Searcher<Map> *search;

	unsigned int width;
	unsigned int height;
	std::vector<std::vector<FlowField>> fields;

	FlowFields() {
	}

	int sectorIdx(int x, int y) {
		int px = x / PER_SECTOR;
		int py = y / PER_SECTOR;
		return px + py * this->width;
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

	int onSectorBorder(int x, int y) {
		int border = None;
		if (x == 0)
			border = Left;
		if (y == 0)
			border = Top;
		if (x == PER_SECTOR - 1)
			border = Right;
		if (y == PER_SECTOR - 1)
			border = Bottom;

		return border;
	}

	int onBorder(int x, int y) {
		int grx = x % PER_SECTOR;
		int gry = y % PER_SECTOR;
		return this->onSectorBorder(grx, gry);
	}

	void markUpdate(int x, int y)
	{
		markFlowFieldsUpdate.insert(this->sectorIdx(x, y));
	}

	void applyUpdateSectors() {
		for (int sectorIdx : markFlowFieldsUpdate) {
			this->updateSector(sectorIdx);
		}
		markFlowFieldsUpdate.clear();
	}

	void updateSector(int idx) {
#ifdef FLOWFIELDS_DEBUG
		clock.restart();
		sf::Time elapsed1 = clock.getElapsedTime();
#endif
		for (int dy = 0; dy < PER_SECTOR; ++dy) {
			for (int dx = 0; dx < PER_SECTOR; ++dx) {
				FlowField &field = fields[idx][dx + dy * PER_SECTOR];
				field.build(sf::Vector2i(dx, dy));
			}
		}

#ifdef FLOWFIELDS_DEBUG
		sf::Time elapsed2 = clock.getElapsedTime();
		std::cout << "FlowFields update sector " <<  idx << " " << (elapsed2 - elapsed1).asSeconds() << std::endl;
#endif
	}

	void initSector(int x, int y) {
		int idx = x + y * this->width;
#ifdef FLOWFIELDS_DEBUG
		clock.restart();
		sf::Time elapsed1 = clock.getElapsedTime();
#endif
		for (int dy = 0; dy < PER_SECTOR; ++dy) {
			for (int dx = 0; dx < PER_SECTOR; ++dx) {
				fields[idx].push_back(FlowField());
				FlowField &field = fields[idx].back();

				field.setGrid(map, sf::IntRect(x * PER_SECTOR, y * PER_SECTOR, PER_SECTOR, PER_SECTOR));
				field.build(sf::Vector2i(dx, dy));
			}
		}

#ifdef FLOWFIELDS_DEBUG
		sf::Time elapsed2 = clock.getElapsedTime();
		std::cout << "FlowFields init sector " <<  idx << " " << (elapsed2 - elapsed1).asSeconds() << std::endl;
#endif
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
				this->initSector(x, y);
			}
		}
	}

	bool pathAvailable(sf::Vector2i pos) {
		return this->map->pathAvailable(pos.x, pos.y) && this->map->dynamicPathfinding.get(pos.x, pos.y) == 0;
	}

	bool pathAvailablePrevision(sf::Vector2i pos) {
		return this->map->movingPathfinding.get(pos.x, pos.y) == 0;
	}

};

class FlowFieldPathFinder {
	FlowFields *flowFields;
	std::vector<SectorPathPosition> sectorPaths;
	std::vector<BorderTransition> transitions;
	std::map<int, BorderTransition> transitionsMap;
	std::list<sf::Vector2i> prevPos;
	sf::Vector2i curPos;
	sf::Vector2i destPos;
	bool found;
public:
	FlowFieldPathFinder() {
		flowFields = nullptr;
//		prevPos = sf::Vector2i(-1,-1);
		curPos = sf::Vector2i(-1, -1);
		destPos = sf::Vector2i(-1, -1);
	}

	void setFlowFields(FlowFields *f) {
		flowFields = f;
	}

#ifdef PATHFINDING_FLOWFIELD_DYNAMIC
	FlowField currentFlowField;

	FlowField *getCurrentFlowField(int cx, int cy, int dx, int dy) {
		bool ffFound = false;
		int curPartIdx = this->flowFields->sectorIdx(cx, cy);
		int partIdx = this->flowFields->sectorIdx(dx, dy);
		int curGrIdx = this->flowFields->gridIdx(cx, cy);
		int grIdx = this->flowFields->gridIdx(dx, dy);

		sf::Vector2i ndpos(dx % PER_SECTOR, dy % PER_SECTOR);

		int fx = cx / (PER_SECTOR);
		int fy = cy / (PER_SECTOR);
		this->currentFlowField.setGrid(flowFields->map, sf::IntRect(fx * PER_SECTOR, fy * PER_SECTOR, PER_SECTOR, PER_SECTOR));

		if (partIdx == curPartIdx) {

//			std::cout << "FlowFieldPathFinder at " << partIdx << " " << grIdx << std::endl;
//			this->currentFlowField = flowFields->fields[partIdx][grIdx];
			ffFound = true;
		} else {
			for (BorderTransition trans : transitions) {
				if (trans.partIdx == curPartIdx) {
					int lIdx = this->flowFields->gridIdx(trans.sPos.x, trans.sPos.y);
#ifdef FLOWFIELDS_DEBUG
					std::cout << "FlowFieldPathFinder traverse dest " << trans.sPos.x << "x" << trans.sPos.y << " " << lIdx << std::endl;
#endif
//					this->currentFlowField = flowFields->fields[curPartIdx][lIdx];
					ndpos.x = trans.sPos.x % PER_SECTOR;
					ndpos.y = trans.sPos.y % PER_SECTOR;
					ffFound = true;
					break;
				}
			}
		}
		if (!ffFound) {
#ifdef FLOWFIELDS_DEBUG
			std::cout << "BUG: cannot found flowfield " << partIdx << "/" << curPartIdx << " " << cx << "x" << cy << "->" << dx << "x" << dy << std::endl;;
			for (BorderTransition trans : transitions) {
				std::cout << "BUG transition " << trans.sPos.x << "x" << trans.sPos.y << " " << trans.partIdx << std::endl;
			}
#endif
//			std::vector<FlowField> &sectorFields = flowFields->fields[curPartIdx];
//			std::cout << "FlowFieldPathFinder (fix) at " << curPartIdx << " " << curGrIdx << std::endl;
//			flowField = &sectorFields[curGrIdx];
		}

		if (ffFound) {
			this->currentFlowField.build(ndpos);
			return &this->currentFlowField;
		} else {
			return nullptr;
		}
	}
#else
	FlowField *getCurrentFlowField(int cx, int cy, int dx, int dy) {
		FlowField *flowField = nullptr;
		int curPartIdx = this->flowFields->sectorIdx(cx, cy);
		int partIdx = this->flowFields->sectorIdx(dx, dy);
		int curGrIdx = this->flowFields->gridIdx(cx, cy);
		int grIdx = this->flowFields->gridIdx(dx, dy);

		if (partIdx == curPartIdx) {
			std::vector<FlowField> &sectorFields = flowFields->fields[partIdx];
//			std::cout << "FlowFieldPathFinder at " << partIdx << " " << grIdx << std::endl;
			flowField = &sectorFields[grIdx];
		} else {
			for (BorderTransition trans : transitions) {
				if (trans.partIdx == curPartIdx) {
					int lIdx = this->flowFields->gridIdx(trans.sPos.x, trans.sPos.y);
#ifdef FLOWFIELDS_DEBUG
					std::cout << "FlowFieldPathFinder traverse dest " << trans.sPos.x << "x" << trans.sPos.y << " " << lIdx << std::endl;
#endif
					flowField = &flowFields->fields[curPartIdx][lIdx];
					break;
				}
			}
		}
		if (!flowField) {
#ifdef FLOWFIELDS_DEBUG
			std::cout << "BUG: cannot found flowfield " << partIdx << "/" << curPartIdx << " " << cx << "x" << cy << "->" << dx << "x" << dy << std::endl;;
			for (BorderTransition trans : transitions) {
				std::cout << "BUG transition " << trans.sPos.x << "x" << trans.sPos.y << " " << trans.partIdx << std::endl;
			}
#endif
//			std::vector<FlowField> &sectorFields = flowFields->fields[curPartIdx];
//			std::cout << "FlowFieldPathFinder (fix) at " << curPartIdx << " " << curGrIdx << std::endl;
//			flowField = &sectorFields[curGrIdx];
		}

		return flowField;
	}
#endif

	sf::Vector2i next(int cx, int cy, int dx, int dy) {
		sf::Vector2i cpos(cx, cy);
		if (curPos != cpos) {
			prevPos.push_back(curPos);
			if (prevPos.size() > 4)
				prevPos.pop_front();
			curPos = cpos;
		}
		sf::Vector2i cgpos = this->flowFields->gridPos(cx, cy);

		int curPartIdx = this->flowFields->sectorIdx(cx, cy);
		int partIdx = this->flowFields->sectorIdx(dx, dy);
		int grIdx = this->flowFields->gridIdx(dx, dy);

#ifdef FLOWFIELDS_DEBUG
		std::cout << "FlowFieldPathFinder next " << cx << "x" << cy << " " << dx << "x" << dy << std::endl;
#endif
		sf::Vector2i npos(cx, cy);
		// on same part
		FlowField *flowField = this->getCurrentFlowField(cx, cy, dx, dy);
		if (flowField) {
			npos = flowField->next(cgpos);

			if (partIdx != curPartIdx) {
				int border = this->flowFields->onSectorBorder(cgpos.x, cgpos.y);
				if (border) {

					if (transitionsMap.count(curPartIdx)) {
						BorderTransition trans = transitionsMap[curPartIdx];
						if (trans.border == border) {
#ifdef FLOWFIELDS_DEBUG
							std::cout << "FlowFieldPathFinder ON BORDER " << trans.dir.x << "x" << trans.dir.y << std::endl;;
#endif
							npos.x = cgpos.x + trans.dir.x;
							npos.y = cgpos.y + trans.dir.y;
						}
					}
				}
#ifdef FLOWFIELDS_DEBUG
				std::cout << "FlowFieldPathFinder need to pass through " << partIdx << std::endl;
#endif
			}

			sf::Vector2i spos = this->flowFields->sectorPos(cx, cy) * PER_SECTOR;
			npos += spos;
#ifdef FLOWFIELDS_DEBUG
			std::cout << "FlowFieldPathFinder nextpos " << npos.x << "x" << npos.y << std::endl;
#endif
		}

		if (!flowField || !this->flowFields->pathAvailable(npos)) {
#ifdef FLOWFIELDS_DEBUG
			std::cout << "FlowFieldPathFinder nextpos NOT AVAILABLE " << npos.x << "x" << npos.y << std::endl;
#endif
			sf::Vector2i apos(cx, cy);
			float distance = std::numeric_limits<float>::max();

			for (int x = curPos.x - 1; x < curPos.x + 2; ++x) {
				for (int y = curPos.y - 1; y < curPos.y + 2; ++y) {
					sf::Vector2i tpos(x, y);
					bool traversed = false;
					for (sf::Vector2i ppos : prevPos) {
						if (tpos == ppos)
							traversed = true;
					}
					if (!traversed && tpos != curPos && tpos != npos && this->flowFields->pathAvailable(tpos)) {
						sf::Vector2i direction = destPos - tpos;
						float tdist = vectorLength(sf::Vector2f(direction));
						if (tdist < distance) {
							apos = tpos;
							distance = tdist;
						}
					}
				}
			}
			npos = apos;
		}

		// check if somebody is coming here
//		if(!this->flowFields->pathAvailablePrevision(npos)) {
//			npos = cpos;
//		}

		return npos;
	}


	bool startFindPath(int sx, int sy, int dx, int dy) {
		if (sf::Vector2i(dx, dy) != destPos) {
			prevPos.clear();
			destPos = sf::Vector2i(dx, dy);
			sectorPaths.clear();
			transitions.clear();
			transitionsMap.clear();
#ifdef FLOWFIELDS_DEBUG
			std::cout << "FlowFieldPathFinder findPath " << sx << "x" << sy << " -> " << dx << "x" << dy << std::endl;
#endif
			JPS::PathVector path;
			this->found = flowFields->search->findPath(path, JPS::Pos(sx, sy), JPS::Pos(dx, dy), 1);
			int sPartIdx = this->flowFields->sectorIdx(sx, sy);
			int sBorder = this->flowFields->onBorder(sx, sy);

			if (sBorder) {
#ifdef FLOWFIELDS_DEBUG
				std::cout << "FlowFieldPathFinder should traverse (source) " << sPartIdx << " -> " << sx << "x" << sy << std::endl;
#endif
				sectorPaths.push_back(SectorPathPosition {sx, sy, sPartIdx, sBorder});
			}

			if (this->found) {
				// find border pos
				for (auto &p : path) {
					sf::Vector2i vp((int)p.x, (int)p.y);
					int partIdx = this->flowFields->sectorIdx(vp.x, vp.y);
					int border = this->flowFields->onBorder(vp.x, vp.y);
					if (border)
					{
						if (sectorPaths.size() > 0) {
							SectorPathPosition prevPos = sectorPaths.back();
							if (prevPos.partIdx != partIdx) {
								sf::Vector2i dir(vp.x - prevPos.x, vp.y - prevPos.y);
#ifdef FLOWFIELDS_DEBUG
								std::cout << "FlowFieldPathFinder prev " << prevPos.partIdx << " -> " << partIdx << " " << prevPos.x << "x" << prevPos.y << " " << vp.x << "x" << vp.y << " " << dir.x << "x" << dir.y << std::endl;
#endif
								transitions.push_back(BorderTransition {prevPos.partIdx, prevPos.border, dir, sf::Vector2i(prevPos.x, prevPos.y)});
								transitionsMap[prevPos.partIdx] = BorderTransition{prevPos.partIdx, prevPos.border, dir, sf::Vector2i(prevPos.x, prevPos.y)};
							}
						}
#ifdef FLOWFIELDS_DEBUG
						std::cout << "FlowFieldPathFinder should traverse " << partIdx << " -> " << vp.x << "x" << vp.y << std::endl;
#endif
						sectorPaths.push_back(SectorPathPosition {vp.x, vp.y, partIdx, border});
					} else {
//						std::cout << "FlowFields should traverse (ignored) " << partIdx << " -> " << vp.x << "x" << vp.y << std::endl;
					}
				}

			}
			else {
#ifdef FLOWFIELDS_DEBUG
				std::cout << "FlowFieldPathFinder path not found " << sx << "x" << sy << " -> " << dx << "x" << dy << std::endl;
#endif
			}

		} else {
			// check if destination position is still available
			if (this->found)
				this->found = this->flowFields->pathAvailable(destPos);
		}
		return this->found;
	}

};
