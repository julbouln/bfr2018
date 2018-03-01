#pragma once

#include "third_party/JPS.h"
#include "third_party/dbscan/dbscan.h"

#include "GameSystem.hpp"
#include "FlowField.hpp"
#include "Steering.hpp"

#define PATHFINDING_MAX_NO_PATH 8
#define STEERING_RADIUS 3

class PathfindingSystem : public GameSystem {
#ifndef PATHFINDING_FLOWFIELD
	JPS::Searcher<Map> *search;
#endif

public:
	FlowFieldPathFind flowFieldPathFind;

	PathfindingSystem() {
#ifndef PATHFINDING_FLOWFIELD
		search = nullptr;
#endif
	}

	~PathfindingSystem() {
#ifndef PATHFINDING_FLOWFIELD
		if (search)
			delete search;
#endif
	}

	void init() {
#ifndef PATHFINDING_FLOWFIELD
		search = new JPS::Searcher<Map>(*this->map);
#endif
//		this->testDbscan();
		flowFieldPathFind.init(this->map);
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
		}

		for (auto pair : clusters) {
			std::cout << "DBSCAN cluster " << pair.first << " " << pair.second.size() << std::endl;
		}

	}

	void updatePathfindingLayer(float dt) {
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

	void updateQuadtrees() {
		this->map->units->clear();
		auto unitView = this->vault->registry.persistent<Tile, Unit>();

		for (EntityID entity : unitView) {
			Tile &tile = unitView.get<Tile>(entity);
			this->map->units->add(QuadtreeObject(entity, tile.ppos.x - 16.0f, tile.ppos.y - 16.0f, 32.0f, 32.0f));
//			this->map->units->add(QuadtreeObject{entity, tile.ppos.x, tile.ppos.y, 32.0f, 32.0f});
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
						std::cout << "Pathfinding: " << other << " go to " << npos << std::endl;
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

	void updateSteering(float dt) {
		this->updateQuadtrees();
		auto view = this->vault->registry.persistent<Tile, GameObject, Unit>();
		for (EntityID entity : view) {
			Tile &tile = view.get<Tile>(entity);
			GameObject &obj = view.get<GameObject>(entity);
			Unit &unit = view.get<Unit>(entity);

			Steering steering;

			if (obj.life > 0)
			{
				tile.pos = sf::Vector2i(trunc(tile.ppos / 32.0f)); // trunc map pos

//				if (tile.pos == unit.destpos) {
//					unit.velocity = sf::Vector2f(0, 0);
//				}

				SteeringObject curSteerObj = SteeringObject{entity, tile.ppos, unit.velocity, unit.speed, 0.5f};

				if (tile.pos != unit.pathPos) {
					this->map->objs.set(unit.pathPos.x, unit.pathPos.y, 0); // mark pos immediatly
					this->map->objs.set(tile.pos.x, tile.pos.y, entity); // mark pos immediatly

					unit.pathUpdate = true;
					unit.pathPos = tile.pos;
				}

				std::vector<SteeringObject> surroundingObjects = this->getSurroundingSteeringObjects(entity, tile.pos.x, tile.pos.y);

				sf::Vector2f steerForce(0, 0);
				/*
									sf::Vector2f cpPos = sf::Vector2f(tile.pos) * 32.0f + 16.0f;
									bool nobodyMove = true;
									for (auto &mo : steering.objects) {
										if (mo.velocity != sf::Vector2f(0, 0))
											nobodyMove = false;
									}
									if (nobodyMove && unit.velocity == sf::Vector2f(0, 0) && (tile.state == "idle" || tile.state == "move")) {
										if (round(tile.ppos) != round(cpPos)) {
											steerForce = steering.seek(curSteerObj, cpPos);
											tile.view = this->getDirection(sf::Vector2i(normalize(steerForce) * 4.0f));
											tile.state = "move";
										}
									}
				*/
				int casesSight = 1;

				std::vector<sf::Vector2f> cases;
				for (int cx = tile.pos.x - casesSight; cx <= tile.pos.x + casesSight; ++cx) {
					for (int cy = tile.pos.y - casesSight; cy <= tile.pos.y + casesSight; ++cy) {
						if (this->map->bound(cx, cy)) {
							if (!this->map->pathAvailable(cx, cy)) {
								cases.push_back(sf::Vector2f(cx * 32.0f, cy * 32.0f));
							}
						}
					}
				}

				sf::Vector2i nextPos = tile.pos + unit.direction;
				sf::Vector2f center = sf::Vector2f(nextPos) * 32.0f + 16.0f;

//					sf::Vector2f wallsForce = steering.repulsionFromWalls(curSteerObj, cases);

				sf::Vector2f accel(0, 0);
				accel += steerForce;
				if(tile.pos == unit.destpos) {
				accel += steering.arrive(curSteerObj, center);
//					accel += steering.seek(curSteerObj, center, 1.0f);
				} else {
				accel += steering.followFlowField(curSteerObj, unit.direction);
				}
//					vel += avoidForce;
				accel += steering.avoid(curSteerObj, cases);
				accel += steering.separate(curSteerObj, surroundingObjects);
//					vel += steering.align(curSteerObj, steering.objects);
//					vel += steering.cohesion(curSteerObj, steering.objects);

//					vel += steering.flock(curSteerObj, steering.objects);



//					std::cout << "UNIT ACCEL" << entity << " "<<accel << std::endl;

//					std::cout << "UNIT VELOCITY (before)" << entity << " "<<unit.velocity << std::endl;
				unit.velocity = unit.velocity + accel;
				unit.velocity = limit(unit.velocity, curSteerObj.maxSpeed);
//					std::cout << "UNIT VELOCITY (after)" << entity << " "<<unit.velocity << std::endl;
				tile.ppos += unit.velocity;

//					if (tile.state != "attack" && tile.ppos == cpPos) {
//						tile.state = "idle";
//					}


				if (tile.state != "attack") {
					if (length(unit.velocity) < 0.5f) {
						this->changeState(entity, "idle");
					} else {
						tile.view = this->getDirection(sf::Vector2i(normalize(unit.velocity) * 16.0f));
						this->changeState(entity, "move");
					}
				}
			}
		}
	}

	void update(float dt) {
		this->updatePathfindingLayer(dt);

		auto view = this->vault->registry.persistent<Tile, GameObject, Unit>();


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

		for (EntityID entity : view) {
			Tile &tile = view.get<Tile>(entity);
			GameObject &obj = view.get<GameObject>(entity);
			Unit &unit = view.get<Unit>(entity);

			if (obj.life > 0 && unit.pathUpdate) {
				unit.pathUpdate = false;
				//this->unitInCase(unit, tile)) {
				this->map->objs.set(tile.pos.x, tile.pos.y, entity); // mark pos immediatly
//				tile.ppos = sf::Vector2f(tile.pos) * (float)32.0;

				if (tile.pos != unit.destpos) {

#ifdef PATHFINDING_FLOWFIELD
					unit.flowFieldPath.setPathFind(&flowFieldPathFind);
					bool found = unit.flowFieldPath.start(tile.pos.x, tile.pos.y, unit.destpos.x, unit.destpos.y);
#else
					JPS::PathVector path;
//						bool found = JPS::findPath(path, *this->map, tile.pos.x, tile.pos.y, unit.destpos.x, unit.destpos.y, 1);g
					bool found = search->findPath(path, JPS::Pos(tile.pos.x, tile.pos.y), JPS::Pos(unit.destpos.x, unit.destpos.y), 1);
#endif

					if (found)
					{
						sf::Vector2i cpos(tile.pos.x, tile.pos.y);
#ifdef PATHFINDING_FLOWFIELD
						sf::Vector2i npos = unit.flowFieldPath.next(tile.pos.x, tile.pos.y);
#else
						sf::Vector2i npos(path.front().x, path.front().y);
#endif

#ifdef PATHFINDING_DEBUG
						std::cout << "Pathfinding: " << entity << " at " << cpos " next position " << npos << "(" << (npos - cpos) << ")" << std::endl;
#endif

//						if (this->checkAround(entity, npos)) {
						if (npos != cpos) {
#ifdef PATHFINDING_DEBUG
							std::cout << "Pathfinding: " << entity << " check around " << tile.pos << std::endl;
#endif
							unit.nextpos = npos;

//							tile.view = this->getDirection(cpos, npos);
							unit.direction = npos - cpos;
							//this->dirVector2i(tile.view);
//							this->changeState(entity, "move");


						} else {
//							this->changeState(entity, "idle");
							unit.direction = sf::Vector2i(0, 0);

#ifdef PATHFINDING_DEBUG
							std::cout << "Pathfinding: " << entity << " wait a moment " << std::endl;
#endif
						}
					} else {
#ifdef PATHFINDING_DEBUG
						std::cout << "Pathfinding: " << entity << " no path found " << tile.pos << " -> " << unit.destpos << std::endl;
#endif
//						this->changeState(entity, "idle");
						unit.direction = sf::Vector2i(0, 0);
						unit.nopath++;
						unit.reallyNopath++;

						if (unit.reallyNopath > PATHFINDING_MAX_NO_PATH * 4) {
							unit.nopath = 0;
							unit.reallyNopath = 0;
							this->stop(unit);
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
//					this->changeState(entity, "idle");
					unit.direction = sf::Vector2i(0, 0);
				}
			}
		}
	}

private:
	std::vector<SteeringObject> getSurroundingSteeringObjects(EntityID currentEnt, int x, int y) {
		std::vector<SteeringObject> steerObjs;
		float getRadius = 2;
		std::vector<QuadtreeObject> quadObjs = this->map->units->getAt((x - getRadius * 0.5f) * 32.0f, (y - getRadius * 0.5f) * 32.0f, getRadius * 32.0f, getRadius * 32.0f);
		if (quadObjs.size() > 0) {
//		std::cout << "PathfindingSystem: got "<<quadObjs.size()<< " quadtree objects"<<std::endl;
			for (QuadtreeObject &quadObj : quadObjs) {
				if (quadObj.entity != currentEnt) {
//				std::cout << "CALC STEERING FOUND " << quadObj.entity << " at " << quadObj.x << "x" << quadObj.y << std::endl;
					Tile &tile = this->vault->registry.get<Tile>(quadObj.entity);
					Unit &unit = this->vault->registry.get<Unit>(quadObj.entity);
					steerObjs.push_back(SteeringObject{quadObj.entity, tile.ppos, unit.velocity, unit.speed, 0.5f});
				}
			}
		}
		return steerObjs;
	}

};