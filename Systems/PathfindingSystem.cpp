#include "PathfindingSystem.hpp"

PathfindingSystem::PathfindingSystem() {
}

PathfindingSystem::~PathfindingSystem() {
}

void PathfindingSystem::init() {
//		this->testDbscan();
	flowFieldPathFind.init(this->map);
}

void PathfindingSystem::testDbscan() {

	std::vector<int> labels;
	std::vector<Point> points;
	for (int y = 0; y < this->map->height; ++y) {
		for (int x = 0; x < this->map->width; ++x) {
			if (!this->map->pathAvailable(x, y))
				points.push_back(Point{(double)x, (double)y});
		}
	}

	int num = dbscan(points, labels, 1.42, 8);

	std::map<int, std::vector<Point>> clusters;
	std::map<int, int> points_map_size;

	for (int i = 0; i < (int)points.size(); i++) {
		if (clusters.count(labels[i]) == 0)
		{
			clusters[labels[i]] = std::vector<Point>();
			clusters[labels[i]].push_back(points[i]);
		} else {
			clusters[labels[i]].push_back(points[i]);
		}
	}

	for (auto pair : clusters) {
		std::cout << "DBSCAN cluster " << pair.first << " " << pair.second.size() << std::endl;
	}
}

void PathfindingSystem::updatePathfindingLayer(float dt) {
	auto buildingView = this->vault->registry.persistent<Tile, Building>();
	this->map->pathfinding.clear();

	for (EntityID entity : buildingView) {
		Tile &tile = buildingView.get<Tile>(entity);

		for (sf::Vector2i const &p : this->tileSurface(tile)) {

			this->map->pathfinding.set(p.x, p.y, entity);
		}
	}

	auto decorView = this->vault->registry.persistent<Tile, Decor>();
	for (EntityID entity : decorView) {
		Tile &tile = decorView.get<Tile>(entity);
		Decor &decor = decorView.get<Decor>(entity);

		if (decor.blocking) {
			for (sf::Vector2i const &p : this->tileSurface(tile)) {
				this->map->pathfinding.set(p.x, p.y, entity);
			}
		}
	}
}

void PathfindingSystem::update(float dt) {
	this->updatePathfindingLayer(dt);
	auto view = this->vault->registry.persistent<Tile, GameObject, Unit>();

#if 0
	// avoid multiple unit at the same pos
	for (EntityID entity : view) {
		Tile &tile = view.get<Tile>(entity);
		GameObject &obj = view.get<GameObject>(entity);
		Unit &unit = view.get<Unit>(entity);

//			if (tile.pos != unit.destpos && vectorLength(unit.destpos - tile.pos) < 1.5 && this->map->objs.get(unit.destpos.x, unit.destpos.y) != entity && unit.velocity == sf::Vector2f(0.0, 0.0)) {
//				unit.destpos = this->firstAvailablePosition(unit.destpos, 1, 16);
//			}

		if (tile.pos == unit.destpos) {
			EntityID samePosEnt = this->map->objs.get(tile.pos.x, tile.pos.y);
			if (samePosEnt != 0 && samePosEnt != entity) {
				if (this->vault->registry.has<Unit>(samePosEnt)) {
					Tile &samePosTile = this->vault->registry.get<Tile>(samePosEnt);
					Unit &samePosUnit = this->vault->registry.get<Unit>(samePosEnt);
					if (samePosTile.pos == samePosUnit.destpos)
					{
#ifdef PATHFINDING_DEBUG
						std::cout << "Pathfinding: " << entity << " at same pos than " << samePosEnt << " move " << tile.pos << std::endl;
#endif
//							unit.destpos = this->firstAvailablePosition(tile.pos, 1, 16);
						this->goTo(unit, tile.pos);
						/*							for (sf::Vector2i const &p : this->tileAround(tile, 1, 1)) {
														if (this->map->pathAvailable(p.x,p.y) && this->map->objs.get(p.x, p.y)==0) {
															unit.destpos = p;
															break;
														}
													}
													*/
					}
				}
			}
		}
	}
#endif

	for (EntityID entity : view) {
		Tile &tile = view.get<Tile>(entity);
		GameObject &obj = view.get<GameObject>(entity);
		Unit &unit = view.get<Unit>(entity);

		if (obj.life > 0 && unit.pathUpdate) {
			unit.pathUpdate = false;
			this->map->objs.set(tile.pos.x, tile.pos.y, entity); // mark pos immediatly

			if (tile.pos != unit.destpos) {
				unit.flowFieldPath.setPathFind(&flowFieldPathFind);
				bool found = unit.flowFieldPath.start(tile.pos.x, tile.pos.y, unit.destpos.x, unit.destpos.y);

				if (found)
				{
					sf::Vector2i cpos(tile.pos.x, tile.pos.y);
					sf::Vector2i npos = unit.flowFieldPath.next(tile.pos.x, tile.pos.y);

#ifdef PATHFINDING_DEBUG
					std::cout << "Pathfinding: " << entity << " at " << cpos << " next position " << npos << "(" << (npos - cpos) << ")" << std::endl;
#endif

					if (npos != cpos) {
						unit.direction = npos - cpos;
					} else {
						unit.direction = sf::Vector2i(0, 0);

#ifdef PATHFINDING_DEBUG
						std::cout << "Pathfinding: " << entity << " wait a moment " << std::endl;
#endif
					}
				} else {
#ifdef PATHFINDING_DEBUG
					std::cout << "Pathfinding: " << entity << " no path found " << tile.pos << " -> " << unit.destpos << std::endl;
#endif
					unit.direction = sf::Vector2i(0, 0);
					unit.nopath++;
					unit.reallyNopath++;

					if (unit.reallyNopath > PATHFINDING_MAX_NO_PATH * 4) {
						unit.nopath = 0;
						unit.reallyNopath = 0;
						unit.destpos = tile.pos;
						unit.commanded = false;
					}

					if (unit.nopath > PATHFINDING_MAX_NO_PATH) {
						sf::Vector2i fp = this->firstAvailablePosition(unit.destpos, 1, 16);
#ifdef PATHFINDING_DEBUG
						std::cout << "Pathfinding: " << entity << " go to first free position " << fp << std::endl;
#endif
						this->goTo(unit, fp);
					}

				}

			} else {
#ifdef PATHFINDING_DEBUG
				std::cout << "Pathfinding: " << entity << " at destination" << std::endl;
#endif
				unit.direction = sf::Vector2i(0, 0);
				unit.commanded = false;
			}
		}
	}
}
