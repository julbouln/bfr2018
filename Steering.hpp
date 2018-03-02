#pragma once

#include "Config.hpp"

#define MAX_SEE_AHEAD 32.0f
#define MAX_AVOID_FORCE 1.0f
#define OBJ_RADIUS 20.0f

#define MAX_QUEUE_AHEAD 32.0f
#define MAX_QUEUE_RADIUS 16.0f

#define AVOID_DIST 48.0f
#define SEPARATION_DIST 24.0f
#define COHESION_DIST 32.0f
#define ALIGN_DIST 32.0f

struct SteeringObject {
	EntityID entity;
	sf::Vector2f pos;
	sf::Vector2f velocity;
	float maxSpeed;
	float maxForce;
};

class Steering {
public:

	sf::Vector2f seek(SteeringObject &currentObject, sf::Vector2f dpos) {
		sf::Vector2f steer = normalize(sf::Vector2f(dpos - currentObject.pos)) * currentObject.maxSpeed;
#ifdef STEERING_DEBUG
		if (steer != sf::Vector2f(0, 0))
			std::cout << "Steering: seek " << currentObject.entity << " " << steer << std::endl;
#endif
		steer -= currentObject.velocity;
		return limit(steer, currentObject.maxForce);
	}

	sf::Vector2f seek(SteeringObject &currentObject, sf::Vector2f dpos, float speed) {
		sf::Vector2f steer = normalize(sf::Vector2f(dpos - currentObject.pos)) * speed;
#ifdef STEERING_DEBUG
		if (steer != sf::Vector2f(0, 0))
			std::cout << "Steering: seek " << currentObject.entity << " " << steer << std::endl;
#endif
		steer -= currentObject.velocity;
		return limit(steer, currentObject.maxForce);
	}



	sf::Vector2f arrive(SteeringObject &currentObject, sf::Vector2f target) {
		float range = 24.0f;
		sf::Vector2f steer = target - currentObject.pos;

		float d = length(steer);
		steer = normalize(steer);

		if (d < range) {
			float m = (currentObject.maxSpeed / range) * d;
			steer *= m;
		} else {
			steer *= currentObject.maxSpeed;
		}

		steer -= currentObject.velocity;
		return limit(steer, currentObject.maxForce);
	}



	sf::Vector2f flee(SteeringObject &currentObject, sf::Vector2f dpos) {
		sf::Vector2f steer = normalize(sf::Vector2f(currentObject.pos - dpos)) * currentObject.maxSpeed;
#ifdef STEERING_DEBUG
		if (steer != sf::Vector2f(0, 0))
			std::cout << "Steering: flee " << currentObject.entity << " " << steer << std::endl;
#endif
		return steer;
	}

	sf::Vector2f followFlowField(SteeringObject &currentObject, sf::Vector2i direction) {
		sf::Vector2f steer = normalize(sf::Vector2f(direction)) * currentObject.maxSpeed;
		steer -= currentObject.velocity;
		return limit(steer, currentObject.maxForce);
	}

	sf::Vector2f followPath(SteeringObject &currentObject, std::vector<sf::Vector2f> path, int idx) {
		int i = 0, index = 0;
		float dist = std::numeric_limits<float>::max();

		for (sf::Vector2f vect : path)
		{
			sf::Vector2f temp = (vect - currentObject.pos);
			float len = fabs(length(temp));
			if (len < dist)
			{
				index = i;
				dist = len;
			}
			i++;
		}

#ifdef STEERING_DEBUG
		std::cout << "Steering: followPath index(1):" << index << std::endl;
#endif
		// arrive on last pos
		if (index == path.size() - 1)
		{
#ifdef STEERING_DEBUG
			std::cout << "Steering: followPath at destination" << std::endl;
#endif
			return sf::Vector2f(0, 0);
		}

		index++;
#ifdef STEERING_DEBUG
		std::cout << "Steering: followPath index:" << index << std::endl;
#endif

		return seek(currentObject, path[index]);
	}

	// same than separate ?
	sf::Vector2f avoid(SteeringObject &currentObject, std::vector<sf::Vector2f> cases) {
		sf::Vector2f steer(0, 0);
		int count = 0;
		sf::Vector2f pos = currentObject.pos;
		for (sf::Vector2f &c : cases)
		{
			float d = length(currentObject.pos - c);
			if ((d > 0) && (d < AVOID_DIST)) {
				sf::Vector2f diff = normalize(currentObject.pos - c) / d;

				steer += diff;
				count++;
			}
		}
		if (count > 0) {
			steer /= (float)count;
			steer = normalize(steer);
			steer *= currentObject.maxSpeed;

			steer -= currentObject.velocity;
			steer = limit(steer, currentObject.maxForce);
			return steer;
		} else {
			return sf::Vector2f(0, 0);
		}
	}

	sf::Vector2f separate(SteeringObject &currentObject, std::vector<SteeringObject> &others) {
		sf::Vector2f steer(0, 0);
		int count = 0;
		for (SteeringObject &other : others) {
			float d = length(currentObject.pos - other.pos);
			if ((d > 0) && (d < SEPARATION_DIST)) {
				sf::Vector2f diff = normalize(currentObject.pos - other.pos) / d;

				steer += diff;
				count++;
			}
		}
		if (count > 0) {
			steer /= (float)count;
			steer = normalize(steer);
			steer *= currentObject.maxSpeed;

			steer -= currentObject.velocity;
			steer = limit(steer, currentObject.maxForce);

			return steer;
		} else {
			return sf::Vector2f(0, 0);
		}
	}

	sf::Vector2f cohesion(SteeringObject &currentObject, std::vector<SteeringObject> &others) {
		sf::Vector2f steer(0, 0);
		int count = 0;
		for (SteeringObject &other : others) {
			float d = length(currentObject.pos - other.pos);
			if ((d > 0) && (d < COHESION_DIST)) {

				steer += other.pos;
				count++;
			}
		}
		if (count > 0) {
			steer /= (float)count;

			steer -= currentObject.velocity;
			steer = limit(steer, currentObject.maxForce);

			return seek(currentObject, steer);
		} else {
			return sf::Vector2f(0, 0);
		}
	}

	sf::Vector2f align (SteeringObject &currentObject, std::vector<SteeringObject> &others) {
		sf::Vector2f steer(0, 0);
		int count = 0;
		for (SteeringObject &other : others) {
			float d = length(currentObject.pos - other.pos);
			if ((d > 0) && (d < ALIGN_DIST)) {
				steer += other.velocity;
			}
		}
		if (count > 0) {
			steer /= (float)count;
			steer = normalize(steer);
			steer *= currentObject.maxSpeed;

			steer -= currentObject.velocity;
			steer = limit(steer, currentObject.maxForce);

			return steer;
		} else {
			return sf::Vector2f(0, 0);
		}
	}

	sf::Vector2f flock(SteeringObject &currentObject, std::vector<SteeringObject> &others) {
		sf::Vector2f fv(0, 0);
		fv += this->separate(currentObject, others);
		fv += this->align(currentObject, others);
		fv += this->cohesion(currentObject, others);
		return fv;
	}

	SteeringObject* getNeighborAhead(SteeringObject &currentObject, std::vector<SteeringObject> &neighbors) {
		SteeringObject *ret = nullptr;
		sf::Vector2f qa = normalize(currentObject.velocity) * MAX_QUEUE_AHEAD;

		sf::Vector2f ahead = currentObject.pos + qa;

		for (SteeringObject &neighbor : neighbors) {
			float d = length(ahead - neighbor.pos);

			if (d <= MAX_QUEUE_RADIUS) {
				ret = &neighbor;
				break;
			}
		}

		return ret;
	}

	sf::Vector2f queue(SteeringObject &currentObject, std::vector<SteeringObject> &neighbors) {
		SteeringObject *neighbor = this->getNeighborAhead(currentObject, neighbors);

		if (neighbor) {
			sf::Vector2f steer = (currentObject.velocity * 0.3f);

			steer -= currentObject.velocity;
			steer = limit(steer, currentObject.maxForce);

			return steer;
		}

		return sf::Vector2f(0, 0);

	}

	sf::Vector2f queue(SteeringObject &currentObject, std::vector<SteeringObject> &neighbors, sf::Vector2f currentSteer) {
		sf::Vector2f v = currentObject.velocity;
		sf::Vector2f brake;

		SteeringObject *neighbor = this->getNeighborAhead(currentObject, neighbors);

		if (neighbor) {
			brake.x = -currentSteer.x * 0.8f;
			brake.y = -currentSteer.y * 0.8f;
			v *= -1.0f;
			brake += v;

			if (length(currentObject.pos - neighbor->pos) <= MAX_QUEUE_RADIUS) {
				sf::Vector2f steer = (currentObject.velocity * 0.3f);

				steer -= currentObject.velocity;
				steer = limit(steer, currentObject.maxForce);

				return steer;

			}
		}

		return brake;
	}
};