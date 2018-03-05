#include "PathfindingSystem.hpp"

PathfindingSystem::PathfindingSystem() {
#ifndef PATHFINDING_FLOWFIELD
	search = nullptr;
#endif
}

PathfindingSystem::~PathfindingSystem() {
#ifndef PATHFINDING_FLOWFIELD
	if (search)
		delete search;
#endif
}

void PathfindingSystem::init() {
#ifndef PATHFINDING_FLOWFIELD
	search = new JPS::Searcher<Map>(*this->map);
#endif
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

void PathfindingSystem::updateQuadtrees() {
	this->map->units->clear();
	auto unitView = this->vault->registry.persistent<Tile, Unit>();

	for (EntityID entity : unitView) {
		Tile &tile = unitView.get<Tile>(entity);
		Unit &unit = unitView.get<Unit>(entity);
		this->map->units->add(PathfindingObject(entity, tile, unit));
	}

}

void PathfindingSystem::updateSteering(float dt) {
	this->updateQuadtrees();
	auto view = this->vault->registry.persistent<Tile, GameObject, Unit>();
	for (EntityID entity : view) {
		Tile &tile = view.get<Tile>(entity);
		GameObject &obj = view.get<GameObject>(entity);
		Unit &unit = view.get<Unit>(entity);

		if (obj.life > 0)
		{
			tile.pos = sf::Vector2i(trunc(tile.ppos / 32.0f)); // trunc map pos

			PathfindingObject curSteerObj = PathfindingObject(entity, tile, unit);//SteeringObject{entity, tile.ppos, unit.velocity, unit.speed, MAX_FORCE};

			if (tile.pos != unit.pathPos) {
				this->map->objs.set(unit.pathPos.x, unit.pathPos.y, 0); // mark map pos immediatly
				this->map->objs.set(tile.pos.x, tile.pos.y, entity); // mark map pos immediatly

				unit.pathUpdate = true;
				unit.pathPos = tile.pos;
			}

			if (!unit.commanded) {
				unit.destpos = tile.pos;
			}

			std::vector<PathfindingObject> surroundingObjects = this->getSurroundingSteeringObjects(entity, tile.ppos.x, tile.ppos.y);

			std::vector<sf::Vector2f> cases;
			for (int cx = tile.pos.x - OBSTACLE_RADIUS; cx <= tile.pos.x + OBSTACLE_RADIUS; ++cx) {
				for (int cy = tile.pos.y - OBSTACLE_RADIUS; cy <= tile.pos.y + OBSTACLE_RADIUS; ++cy) {
					if (!this->map->bound(cx, cy) || !this->map->pathAvailable(cx, cy)) {
						cases.push_back(sf::Vector2f(cx * 32.0f, cy * 32.0f));
					}
				}
			}

			sf::Vector2f accel(0, 0);

			// escape from obstacle
			if (!this->map->pathAvailable(tile.pos.x, tile.pos.y)) {
				sf::Vector2i bestNextPos = tile.pos;
				sf::Vector2i curDestPos = unit.destpos;
				curDestPos = unit.flowFieldPath.ffDest;
				float dist = std::numeric_limits<float>::max();
				for (sf::Vector2i &fp : this->vectorSurfaceExtended(tile.pos, 1)) {
					if (this->map->pathAvailable(fp.x, fp.y)) {
						if (distance(fp, curDestPos) < dist) {
							dist = distance(fp, curDestPos);
							bestNextPos = fp;
						}
					}
				}
				accel += steering.seek(curSteerObj, sf::Vector2f(bestNextPos * 32) + 16.0f) * 2.0f;
				accel += steering.flee(curSteerObj, sf::Vector2f(tile.pos * 32) + 16.0f);
			}

			accel += steering.avoid(curSteerObj, cases) * 1.0f;

			if (tile.state != "attack") {
				bool seekTarget = false;
				if (unit.targetEnt && this->vault->registry.valid(unit.targetEnt)) {
					Tile &ttile = this->vault->registry.get<Tile>(unit.targetEnt);
					// if attacking and target is near, then seek it and avoid separate
					if (distance(ttile.ppos, tile.ppos) < 64.0f) {
						seekTarget = true;
						accel += steering.seek(curSteerObj, ttile.ppos);
					}
				}

				if (!seekTarget) {
					// check if somebody move arround
					sf::Vector2f sum(0, 0);
					for (auto &obj : surroundingObjects) {
						sum += obj.velocity;
					}

					if (tile.pos == unit.destpos && length(sum) < 0.1f) {
						sf::Vector2f center = sf::Vector2f(tile.pos) * 32.0f + 16.0f;
						accel += steering.arrive(curSteerObj, center);
//					accel += steering.seek(curSteerObj, center, 1.0f);
					} else {
						accel += steering.followFlowField(curSteerObj, unit.direction) * 1.5f;
					}
					accel += steering.separate(curSteerObj, surroundingObjects) * 3.0f;
				}

				// queue if other are already attacking
				/*
				for (SteeringObject &obj : surroundingObjects) {
					Tile &stile = this->vault->registry.get<Tile>(obj.entity);
					if (stile.state == "attack" && obj.entity != unit.targetEnt) {
						accel += steering.queue(curSteerObj, surroundingObjects, accel);
					}
				}
				*/
				sf::Vector2f previousVelocity = unit.velocity;

				unit.velocity = unit.velocity + accel;
				unit.velocity = limit(unit.velocity, curSteerObj.maxSpeed * MIN_VELOCITY, curSteerObj.maxSpeed);
//					std::cout << "UNIT VELOCITY (after)" << entity << " "<<unit.velocity << std::endl;
				tile.ppos += unit.velocity;

#ifdef PATHFINDING_DEBUG
				std::cout << "Pathfinding: " << entity << " steering accel:" << accel << " velocity:" << unit.velocity << " surrounding:" << surroundingObjects.size() << std::endl;
#endif

				float normVelLen = length(normalize(unit.velocity));
				if (normVelLen < 0.2f) {
					unit.velocity = sf::Vector2f(0, 0);
					this->changeState(entity, "idle");
				} else {
//						if (length(unit.velocity) * 1.5f >= unit.speed)
					if (normVelLen >= 1.0f)
						tile.view = this->getDirection(sf::Vector2i(normalize(previousVelocity + unit.velocity) * 16.0f));

					this->changeState(entity, "move");
				}
			} else {
				// face target
				if (unit.targetEnt && this->vault->registry.valid(unit.targetEnt)) {
					Tile &ttile = this->vault->registry.get<Tile>(unit.targetEnt);
					tile.view = this->getDirection(sf::Vector2i(ttile.ppos - tile.ppos));
				}
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
					std::cout << "Pathfinding: " << entity << " at " << cpos << " next position " << npos << "(" << (npos - cpos) << ")" << std::endl;
#endif

					if (npos != cpos) {
#ifdef PATHFINDING_DEBUG
						std::cout << "Pathfinding: " << entity << " check around " << tile.pos << std::endl;
#endif

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
//							this->stop(unit);
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
//					this->changeState(entity, "idle");
				unit.direction = sf::Vector2i(0, 0);
				unit.commanded = false;
			}
		}
	}
}

std::vector<PathfindingObject> PathfindingSystem::getSurroundingSteeringObjects(EntityID currentEnt, float x, float y) {
	std::vector<PathfindingObject> steerObjs;
	std::vector<PathfindingObject> quadObjs = this->map->units->getAt(x - SURROUNDING_RADIUS * 32.0f, y - SURROUNDING_RADIUS * 32.0f, (SURROUNDING_RADIUS + 1) * 32.0f, (SURROUNDING_RADIUS + 1) * 32.0f);
	if (quadObjs.size() > 0) {
//		std::cout << "PathfindingSystem: got "<<quadObjs.size()<< " quadtree objects"<<std::endl;
		for (auto &quadObj : quadObjs) {
			if (quadObj.entity != currentEnt) {
//					Tile &tile = this->vault->registry.get<Tile>(quadObj.entity);
//					Unit &unit = this->vault->registry.get<Unit>(quadObj.entity);
//					steerObjs.push_back(PathfindingObject(quadObj.entity, tile, unit));

				quadObj.update(); // update from pointers
				steerObjs.push_back(quadObj);
			}
		}
	}
	return steerObjs;
}
