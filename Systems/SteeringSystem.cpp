#include "SteeringSystem.hpp"

#define OBSTACLE_RADIUS 1
#define SURROUNDING_RADIUS 1
#define MAX_SURROUNDING_OBJS 64

#define MIN_VELOCITY 0.01f

void SteeringSystem::updateQuadtrees() {
	this->map->units->clear();
	auto unitView = this->vault->registry.persistent<Tile, Unit>();

	for (EntityID entity : unitView) {
		Tile &tile = unitView.get<Tile>(entity);
		Unit &unit = unitView.get<Unit>(entity);
		this->map->units->add(PathfindingObject(entity, tile, unit));
	}

}

void SteeringSystem::update(float dt) {
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
				if (unit.targetEnt) {
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
				}
				accel += steering.separate(curSteerObj, surroundingObjects) * 2.0f;

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
				tile.ppos += unit.velocity;

#ifdef PATHFINDING_DEBUG
				std::cout << "Pathfinding: " << entity << " steering accel:" << accel << " velocity:" << unit.velocity << " surrounding:" << surroundingObjects.size() << std::endl;
#endif

				if (unit.averageCount < 16) {
					unit.averageVelocity += unit.velocity;
					unit.averageCount++;
				} else {
					unit.averageVelocity = unit.velocity;
					unit.averageCount = 1;
				}

				sf::Vector2f avVel = unit.averageVelocity / (float)unit.averageCount;
				float velLen = length(avVel);

				if (velLen < unit.speed * 0.1f) {
					unit.velocity = sf::Vector2f(0, 0);
					this->changeState(entity, "idle");
				} else {
//						if (length(unit.velocity) * 1.5f >= unit.speed)
					if (velLen >= unit.speed * 0.5f)
						tile.view = getDirection(sf::Vector2i(normalize(avVel) * 16.0f));

					this->changeState(entity, "move");
				}
			} else {
				// face target
				if (unit.targetEnt) {
					Tile &ttile = this->vault->registry.get<Tile>(unit.targetEnt);
					tile.view = getDirection(sf::Vector2i(ttile.ppos - tile.ppos));
				}
			}
		}
	}
}

std::vector<PathfindingObject> SteeringSystem::getSurroundingSteeringObjects(EntityID currentEnt, float x, float y) {
	std::vector<PathfindingObject> steerObjs;
	steerObjs.reserve(MAX_SURROUNDING_OBJS);
	std::vector<PathfindingObject> quadObjs;
	quadObjs.reserve(MAX_SURROUNDING_OBJS); // guess we won't have more than MAX_SURROUNDING_OBJS objects in this 3x3 grid
	this->map->units->retrieve(quadObjs, x - SURROUNDING_RADIUS * 32.0f, y - SURROUNDING_RADIUS * 32.0f, (SURROUNDING_RADIUS + 1) * 32.0f, (SURROUNDING_RADIUS + 1) * 32.0f);
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
