#pragma once

#include "third_party/JPS.h"
#include "third_party/dbscan/dbscan.h"

#include "GameSystem.hpp"
#include "FlowField.hpp"
#include "Steering.hpp"

#define PATHFINDING_MAX_NO_PATH 8
#define MOVING_AROUND_SIZE 3

class PathfindingSystem : public GameSystem {
	JPS::Searcher<Map> *search;

public:
	FlowFieldPathFind flowFieldPathFind;

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

//		this->map->dynamicPathfinding.clear();

		auto unitView = this->vault->registry.persistent<Tile, Unit>();
		for (EntityID entity : unitView) {
			Tile &tile = unitView.get<Tile>(entity);
			Unit &unit = unitView.get<Unit>(entity);

			if (tile.pos == unit.destpos) {
//				if(unit.velocity == sf::Vector2f(0,0)) {
//				this->map->dynamicPathfinding.set(tile.pos.x, tile.pos.y, 0);
//				this->map->dynamicPathfinding.set(tile.pos.x, tile.pos.y, 0);
//				this->map->pathfinding.set(tile.pos.x, tile.pos.y, entity);
			} else {
//				this->map->dynamicPathfinding.set(tile.pos.x, tile.pos.y, entity);
//				if(tile.pos != unit.nextpos)
//					this->map->dynamicPathfinding.set(tile.pos.x, tile.pos.y, 0);
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

	void updateSteering(float dt) {
//this->map->dynamicPathfinding.clear();

		auto view = this->vault->registry.persistent<Tile, GameObject, Unit>();
		for (EntityID entity : view) {
			Tile &tile = view.get<Tile>(entity);
			GameObject &obj = view.get<GameObject>(entity);
			Unit &unit = view.get<Unit>(entity);


			Steering steering;
//			this->map->dynamicPathfinding.set(unit.nextpos.x, unit.nextpos.y, entity);

			//if (obj.life > 0 && !this->unitInCase(unit, tile))
			if (obj.life > 0)
			{
				this->calculateSteeringObjects(steering, entity, tile.pos.x, tile.pos.y);

				float speed = (float)unit.speed * 0.5f;
				sf::Vector2f vel = sf::Vector2f(unit.direction) * speed;
				SteeringObject curSteerObj = SteeringObject{entity, tile.ppos, unit.velocity};

				sf::Vector2f sdest = sf::Vector2f(unit.nextpos * 32);
				//			sdest.x+=16.0f;
//				sdest.y+=16.0f;

				vel = steering.seek(curSteerObj, sdest, speed);



//				if (abs(tile.ppos.x / 32.0f - tile.pos.x) > 1 || abs(tile.ppos.y / 32.0f - tile.pos.y) > 1) {
//					sf::Vector2i cpos = sf::Vector2i(tile.ppos / 32.0f);
//					tile.pos = cpos;
				/*					unit.nextpos = cpos;
									SteeringObject curSteerObj = SteeringObject{entity, tile.ppos, vel};
				*/
//					vel = steering.seek(curSteerObj, sf::Vector2f(cpos * 32), 0.5f);

//				} else {
				unit.velocity = vel;

				sf::Vector2f avoid = steering.collisionAvoidance(curSteerObj);
				if (avoid.x != 0 || avoid.y != 0) {
					std::cout << "AVOID " << avoid.x << "x" << avoid.y << " + " << vel.x << "x" << vel.y << std::endl;

					vel += avoid;


					if (unit.destpos == tile.pos) {
						sf::Vector2i pushVec = sf::Vector2i(vectorRound(vectorNormalize(vel * 32.0f)));

						int x = pushVec.x;
						int y = pushVec.y;

						// perpendicular vector
						pushVec.x = -y;
						pushVec.y = x;
						/*
												if (pushVec.x == 0) {
													pushVec.x = pushVec.y;
												} else if (pushVec.y == 0) {
													pushVec.y = pushVec.x;
												} else if (pushVec.x != 0 && abs(pushVec.x) == abs(pushVec.y)) {
													pushVec.x = -pushVec.x;
												}
						*/
						sf::Vector2i pp = tile.pos + pushVec;
//						unit.destpos = pp;
//						unit.nextpos = pp;
//						unit.velocity = vel;
						if (this->map->objs.get(pp.x, pp.y) == 0) {
							unit.destpos = pp;
						} 
/*
						else {
							for (int x = tile.pos.x - 1; x < tile.pos.x + 1; ++x) {
								for (int y = tile.pos.y - 1; y < tile.pos.y + 1; ++y) {
									sf::Vector2i fp(x, y);
									if (this->map->objs.get(x, y) == 0) {
										unit.destpos = fp;
										break;
									}
								}

							}
						}
						*/

					}
				}
//				float speed = (float)unit.speed / 2.0;
//				tile.ppos += this->dirMovement(tile.view, speed);

//				}

				tile.ppos += vel;
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

			if (tile.pos == unit.destpos) {
				EntityID samePosEnt = this->map->objs.get(tile.pos.x, tile.pos.y);
				if (samePosEnt != 0 && samePosEnt != entity) {
					if (this->vault->registry.has<Unit>(samePosEnt)) {
						Tile &samePosTile = this->vault->registry.get<Tile>(samePosEnt);
						Unit &samePosUnit = this->vault->registry.get<Unit>(samePosEnt);
//						if (samePosTile.pos == samePosUnit.destpos)
						{
#ifdef PATHFINDING_DEBUG
							std::cout << "Pathfinding: " << entity << " at same pos than " << samePosEnt << " move " << tile.pos.x << "x" << tile.pos.y << std::endl;
#endif
//							this->goTo(unit, tile.pos);
							for (sf::Vector2i const &p : this->tileAround(tile, 1, 1)) {
								if (this->map->pathAvailable(p.x, p.y) && this->map->dynamicPathfinding.get(p.x, p.y) == 0) {
									unit.destpos = p;
									break;
								}
							}
						}
					}
				}
			}
		}

		for (EntityID entity : view) {
			Tile &tile = view.get<Tile>(entity);
			GameObject &obj = view.get<GameObject>(entity);
			Unit &unit = view.get<Unit>(entity);

			if (obj.life > 0 && tile.pos != unit.destpos && this->unitInCase(unit, tile)) {
//				this->map->dynamicPathfinding.set(tile.pos.x, tile.pos.y, 0);
				tile.pos = unit.nextpos;
//				this->map->dynamicPathfinding.set(tile.pos.x, tile.pos.y, 0);
				this->map->objs.set(tile.pos.x, tile.pos.y, entity); // mark pos immediatly
				tile.ppos = sf::Vector2f(tile.pos) * (float)32.0;

				if (tile.pos != unit.destpos) {

#ifdef PATHFINDING_FLOWFIELD
					unit.flowFieldPath.setPathFind(&flowFieldPathFind);
					bool found = unit.flowFieldPath.start(tile.pos.x, tile.pos.y, unit.destpos.x, unit.destpos.y);
//					this->calculateMovingObjects(unit.flowFieldPath.getCurrentFlowField(), entity, tile.pos.x, tile.pos.y);
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
						std::cout << "Pathfinding: " << entity << " at " << cpos.x << "x" << cpos.y << " next position " << npos.x << "x" << npos.y << "(" << npos.x - cpos.x << "x" << npos.y - cpos.y << ")" << std::endl;
#endif

//						if (this->checkAround(entity, npos)) {
						if (npos != cpos) {
#ifdef PATHFINDING_DEBUG
							std::cout << "Pathfinding: " << entity << " check around " << tile.pos.x << "x" << tile.pos.y << std::endl;
#endif
//							this->map->pathfinding.set(tile.pos.x, tile.pos.y, 0);
//							this->map->dynamicPathfinding.set(tile.pos.x, tile.pos.y, 0);
							unit.nextpos = npos;
//							this->map->dynamicPathfinding.set(unit.nextpos.x, unit.nextpos.y, entity);

							tile.view = this->getDirection(cpos, npos);
//							unit.velocity = this->dirVelocity(tile.view, unit.speed / 2.0);
							unit.direction = this->dirVector2i(tile.view);
							this->changeState(entity, "move");


						} else {
//							this->map->dynamicPathfinding.set(tile.pos.x, tile.pos.y, 0);
//							this->map->pathfinding.set(tile.pos.x, tile.pos.y, entity);
							this->changeState(entity, "idle");
//							unit.velocity = sf::Vector2f(0, 0);
							unit.direction = sf::Vector2i(0, 0);

#ifdef PATHFINDING_DEBUG
							std::cout << "Pathfinding: " << entity << " wait a moment " << std::endl;
#endif
						}
					} else {
#ifdef PATHFINDING_DEBUG
						std::cout << "Pathfinding: " << entity << " no path found " << tile.pos.x << "x" << tile.pos.y << " -> " << unit.destpos.x << "x" << unit.destpos.y << std::endl;
#endif
//						this->map->dynamicPathfinding.set(tile.pos.x, tile.pos.y, 0);
//						this->map->pathfinding.set(tile.pos.x, tile.pos.y, entity);
						this->changeState(entity, "idle");
//						unit.velocity = sf::Vector2f(0, 0);
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
							std::cout << "Pathfinding: " << entity << " go to first free position " << fp.x << "x" << fp.y << std::endl;
#endif
							this->goTo(unit, fp);
						}

					}

				} else {
#ifdef PATHFINDING_DEBUG
					std::cout << "Pathfinding: " << entity << " at destination" << std::endl;
#endif
//					this->map->dynamicPathfinding.set(tile.pos.x, tile.pos.y, 0);
//					this->map->pathfinding.set(tile.pos.x, tile.pos.y, entity);
					this->changeState(entity, "idle");
//					unit.velocity = sf::Vector2f(0, 0);
					unit.direction = sf::Vector2i(0, 0);
				}
			} else {
//					float speed = (float)unit.speed / 2.0;
//					tile.ppos += this->dirMovement(tile.view, speed);

				// ppos and pos are desynchronized
				if (abs(tile.ppos.x / 32.0f - tile.pos.x) > 1 || abs(tile.ppos.y / 32.0f - tile.pos.y) > 1) {
					// something wrong, realign
					GameObject &obj = this->vault->registry.get<GameObject>(entity);
					std::cout << "Pathfinding: BUG SOMETHING WRONG WITH " << entity << " state:" << tile.state << " life:" << obj.life << " pos:" << tile.pos.x << "x" << tile.pos.y << " nextpos:" << unit.nextpos.x << "x" << unit.nextpos.y << " destpos:" << unit.destpos.x << "x" << unit.destpos.y << std::endl;
//					tile.ppos = sf::Vector2f(tile.pos) * (float)32.0;
					/*
										sf::Vector2i cpos = sf::Vector2i(vectorRound(tile.ppos / 32.0f));
											tile.pos = cpos;
											unit.nextpos = cpos;
											unit.velocity = sf::Vector2f(0, 0);
					*/
					/*

										if (cpos == unit.nextpos) {
											tile.pos = cpos;
											unit.nextpos = cpos;
											unit.velocity = sf::Vector2f(0, 0);
										} else {

											sf::Vector2f nppos = sf::Vector2f(tile.pos * 32);
											sf::Vector2f vel = nppos - tile.ppos;

											unit.velocity = vectorNormalize(vel) * ((float)unit.speed / 2.0f);
											std::cout << "Pathfinding: COMPENSATE " << unit.velocity.x << " " << unit.velocity.y << std::endl;
										}
					*/
				}
			}

		}


	}

private:
	void calculateSteeringObjects(Steering &steering, EntityID currentEnt, int x, int y) {
		steering.objects.clear();
		for (int cx = x - MOVING_AROUND_SIZE; cx < x + MOVING_AROUND_SIZE * 2; ++cx) {
			for (int cy = y - MOVING_AROUND_SIZE; cy < y + MOVING_AROUND_SIZE * 2; ++cy) {
				if (this->map->bound(cx, cy)) {
					EntityID ent = this->map->objs.get(cx, cy);
					if (ent && ent != currentEnt) {
						if (this->vault->registry.has<Unit>(ent)) {
							Tile &tile = this->vault->registry.get<Tile>(ent);
							Unit &unit = this->vault->registry.get<Unit>(ent);
//							if(unit.velocity.x > 0 || unit.velocity.y > 0)
							steering.objects.push_back(SteeringObject{ent, tile.ppos, unit.velocity});
						}
					}
				}
			}
		}
	}

	void calculateMovingObjects(FlowField *flowField, EntityID currentEnt, int x, int y) {
		std::vector<MovingObject> &movingObjects = flowField->movingObjects;
		Tile &curTile = this->vault->registry.get<Tile>(currentEnt);
		Unit &curUnit = this->vault->registry.get<Unit>(currentEnt);
		flowField->currentMovingObjectVel = curUnit.speed / 2.0;
		flowField->currentMovingObject = MovingObject{curTile.ppos, curUnit.velocity};

		movingObjects.clear();
		for (int cx = x - MOVING_AROUND_SIZE; cx < x + MOVING_AROUND_SIZE * 2; ++cx) {
			for (int cy = y - MOVING_AROUND_SIZE; cy < y + MOVING_AROUND_SIZE * 2; ++cy) {
				if (this->map->bound(cx, cy)) {
					EntityID ent = this->map->objs.get(cx, cy);
					if (ent && ent != currentEnt) {
						if (this->vault->registry.has<Unit>(ent)) {
							Tile &tile = this->vault->registry.get<Tile>(ent);
							Unit &unit = this->vault->registry.get<Unit>(ent);
//							if(unit.velocity.x > 0 || unit.velocity.y > 0)
							movingObjects.push_back(MovingObject{tile.ppos, unit.velocity});
						}
					}
				}
			}
		}
	}

};