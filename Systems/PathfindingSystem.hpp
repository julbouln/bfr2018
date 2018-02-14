#pragma once

#include "GameSystem.hpp"
#include "third_party/JPS.h"

#include "FlowField.hpp"

#include "dbscan/dbscan.h"

#define PATHFINDING_MAX_NO_PATH 16

class PathfindingSystem : public GameSystem {
//	Steering steering;
	JPS::Searcher<Map> *search;

public:
	FlowFields flowFields;

	PathfindingSystem() {
		search = nullptr;
	}

	~PathfindingSystem() {
		if (search)
			delete search;
	}

	void init() {
		search = new JPS::Searcher<Map>(*this->map);
//		this->testDbscan();
		flowFields.init(this->map);
	}

	void testDbscan() {

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
//							std::cout << "DBSCAN Point(" << points[i].x << ", " << points[i].y << "): " << labels[i] << std::endl;
			/*
						if (points_map.count(labels[i]) == 0) {
							points_map[labels[i]] = points[i];
							points_map_size[labels[i]] = 1;
						} else {
							points_map[labels[i]].x = points_map[labels[i]].x + points[i].x;
							points_map[labels[i]].y = points_map[labels[i]].y + points[i].y;
							points_map_size[labels[i]] = points_map_size[labels[i]] + 1;
						}
						*/
		}

		for (auto pair : clusters) {
			std::cout << "DBSCAN cluster " << pair.first << " " << pair.second.size() << std::endl;
		}

	}

	void updatePathfindingLayer(float dt) {
		auto buildingView = this->vault->registry.persistent<Tile, Building>();

#ifdef PATHFINDING_FLOWFIELD
		for (EntityID entity : buildingView) {
			Tile &tile = buildingView.get<Tile>(entity);

			for (sf::Vector2i const &p : this->tileSurface(tile)) {
				if (!this->map->pathfinding.get(p.x, p.y)) {
					flowFields.markUpdate(p.x,p.y);
				}
			}
		}
#endif
		this->map->pathfinding.clear();

		for (EntityID entity : buildingView) {
			Tile &tile = buildingView.get<Tile>(entity);

			for (sf::Vector2i const &p : this->tileSurface(tile)) {

				this->map->pathfinding.set(p.x, p.y, entity);
			}
		}

		this->map->dynamicPathfinding.clear();
		this->map->movingPathfinding.clear();

		auto unitView = this->vault->registry.persistent<Tile, Unit>();
		for (EntityID entity : unitView) {
			Tile &tile = unitView.get<Tile>(entity);
			Unit &unit = unitView.get<Unit>(entity);

			if (tile.pos == unit.destpos) {
				this->map->dynamicPathfinding.set(tile.pos.x, tile.pos.y, entity);
			} else {
				this->map->movingPathfinding.set(tile.pos.x, tile.pos.y, entity);				
				this->map->movingPathfinding.set(unit.nextpos.x, unit.nextpos.y, entity);				
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

	bool checkAround(EntityID entity, sf::Vector2i npos) {
		Tile &tile = this->vault->registry.get<Tile>(entity);

		for (sf::Vector2i const &p : this->tileSurfaceExtended(tile, 2)) {
			EntityID other = this->map->objs.get(p.x, p.y);
			if (other && other != entity) {
				if (this->vault->registry.has<Unit>(other)) {
					Unit &otherUnit = this->vault->registry.get<Unit>(other);
					Tile &otherTile = this->vault->registry.get<Tile>(other);
					if (otherTile.pos != otherUnit.destpos && (npos == otherUnit.nextpos || npos == otherTile.pos)) {
#ifdef PATHFINDING_DEBUG
						std::cout << "Pathfinding: " << other << " go to " << npos.x << "x" << npos.y << std::endl;
#endif
						return false;
					}
				}
			}
		}
		return true;
	}

	bool unitInCase(Unit &unit, Tile &tile) {
		int diffx = abs(tile.ppos.x - unit.nextpos.x * 32);
		int diffy = abs(tile.ppos.y - unit.nextpos.y * 32);
		if (diffx >= 0 && diffx <= 2 && diffy >= 0 && diffy <= 2) {
			return true;
		} else {
			return false;
		}
	}

	void updateMovement(float dt) {
		auto view = this->vault->registry.persistent<Tile, GameObject, Unit>();
		for (EntityID entity : view) {
			Tile &tile = view.get<Tile>(entity);
			GameObject &obj = view.get<GameObject>(entity);
			Unit &unit = view.get<Unit>(entity);

			if (obj.life > 0 && !this->unitInCase(unit, tile)) {
//				float speed = (float)unit.speed / 2.0;
//				tile.ppos += this->dirMovement(tile.view, speed);
				tile.ppos += unit.velocity;
			}
		}
	}

#if 0
	void updateSteering(float dt) {
		auto view = this->vault->registry.persistent<Tile, GameObject, Unit>();
		for (EntityID entity : view) {
			Tile &tile = view.get<Tile>(entity);
			GameObject &obj = view.get<GameObject>(entity);
			Unit &unit = view.get<Unit>(entity);

			switch (unit.steeringState) {
			case SteeringState::Pursue: {
				if (this->unitInCase(unit, tile)) {
					tile.pos = unit.nextpos;
					if (unit.targetEnt && this->vault->registry.valid(unit.targetEnt)) {
						Tile &destTile = this->vault->registry.get<Tile>(unit.targetEnt);

						sf::Vector2f velocity;
						if (this->vault->registry.has<Unit>(unit.targetEnt)) {
							Unit &destUnit = this->vault->registry.get<Unit>(unit.targetEnt);
							velocity = steering.pursue(tile, destTile, unit, destUnit );
						} else {
							velocity = steering.seek(tile, destTile);
						}

						if (vectorLength(velocity) > 0) {
							sf::Vector2i nextpos = tile.pos + sf::Vector2i(vectorRound(vectorNormalize(velocity)));
							if (destTile.pos != nextpos) {
								unit.nextpos = nextpos;
								unit.velocity = velocity;
								tile.view = this->getDirection(tile.pos, tile.pos + sf::Vector2i(unit.velocity));
								this->changeState(entity, "move");
								std::cout << "Steering pursue " << entity << " " << tile.pos.x << "x" << tile.pos.y << " " << unit.nextpos.x << "x" << unit.nextpos.y << std::endl;
							} else {
								unit.steeringState = SteeringState::None;
								this->changeState(entity, "idle");
							}
						}

					}
				}
			}
			break;
			case SteeringState::FollowPath:
				break;
			default:
				break;
			}
		}
	}
#endif
	void update(float dt) {
		this->updatePathfindingLayer(dt);

		auto view = this->vault->registry.persistent<Tile, GameObject, Unit>();
		for (EntityID entity : view) {
			Tile &tile = view.get<Tile>(entity);
			GameObject &obj = view.get<GameObject>(entity);
			Unit &unit = view.get<Unit>(entity);

			if (obj.life > 0 && tile.pos != unit.destpos && this->unitInCase(unit, tile)) {
				tile.pos = unit.nextpos;
				this->map->objs.set(tile.pos.x, tile.pos.y, entity); // mark pos immediatly
				tile.ppos = sf::Vector2f(tile.pos) * (float)32.0;

				if (tile.pos != unit.destpos) {

#ifdef PATHFINDING_FLOWFIELD
					unit.flowFieldPathFinder.setFlowFields(&flowFields);
					bool found = unit.flowFieldPathFinder.startFindPath(tile.pos.x, tile.pos.y, unit.destpos.x, unit.destpos.y);
#else
					JPS::PathVector path;
//						bool found = JPS::findPath(path, *this->map, tile.pos.x, tile.pos.y, unit.destpos.x, unit.destpos.y, 1);
					bool found = search->findPath(path, JPS::Pos(tile.pos.x, tile.pos.y), JPS::Pos(unit.destpos.x, unit.destpos.y), 1);

#endif

					if (found)
					{
						sf::Vector2i cpos(tile.pos.x, tile.pos.y);
#ifdef PATHFINDING_FLOWFIELD
//						sf::Vector2i npos = unit.flowField.next(tile.pos);
						sf::Vector2i npos = unit.flowFieldPathFinder.next(tile.pos.x, tile.pos.y, unit.destpos.x, unit.destpos.y);
#else
						sf::Vector2i npos(path.front().x, path.front().y);
#endif

#ifdef PATHFINDING_DEBUG
						std::cout << "Pathfinding: " << entity << " at " << cpos.x << "x" << cpos.y << " next position " << npos.x << "x" << npos.y << "(" << npos.x - cpos.x << "x" << npos.y - cpos.y << ")" << std::endl;
#endif

#ifdef PATHFINDING_DEBUG
						std::cout << "Pathfinding: " << entity << " check around " << tile.pos.x << "x" << tile.pos.y << std::endl;
#endif
						if (this->checkAround(entity, npos)) {
							unit.nextpos = npos;

							tile.view = this->getDirection(cpos, npos);
							unit.velocity = this->dirVelocity(tile.view, unit.speed / 2.0);
							this->changeState(entity, "move");


						} else {
							this->changeState(entity, "idle");
#ifdef PATHFINDING_DEBUG
							std::cout << "Pathfinding: " << entity << " wait a moment " << std::endl;
#endif
						}
					}

					else {
#ifdef PATHFINDING_DEBUG
						std::cout << "Pathfinding: " << entity << " no path found" << std::endl;
#endif
						this->changeState(entity, "idle");
						unit.nopath++;

						if (unit.nopath > PATHFINDING_MAX_NO_PATH) {
							sf::Vector2i fp = this->firstFreePosition(unit.destpos, this->map->objs, 16);
#ifdef PATHFINDING_DEBUG
							std::cout << "Pathfinding: " << entity << " go to first free position " << fp.x << "x" << fp.y << std::endl;
#endif
							this->goTo(unit, fp);
						}
					}

				} else {
#ifdef PATHFINDING_DEBUG
					std::cout << "Pathfinding: " << entity << " at destination" << std::endl;
#endif
					this->changeState(entity, "idle");
				}
			} else {
//					float speed = (float)unit.speed / 2.0;
//					tile.ppos += this->dirMovement(tile.view, speed);

				if (abs(tile.ppos.x / 32.0 - tile.pos.x) > 1 || abs(tile.ppos.y / 32.0 - tile.pos.y) > 1) {
					// something wrong, realign
					GameObject &obj = this->vault->registry.get<GameObject>(entity);
					std::cout << "Pathfinding: SOMETHING WRONG WITH " << entity << " state:" << tile.state << " life:" << obj.life << " pos:" << tile.pos.x << "x" << tile.pos.y << " nextpos" << unit.nextpos.x << "x" << unit.nextpos.y << " destpos:" << unit.destpos.x << "x" << unit.destpos.y << std::endl;
					tile.ppos = sf::Vector2f(tile.pos) * (float)32.0;
				}
			}

		}

	}
};