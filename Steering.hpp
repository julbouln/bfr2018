#pragma once

#include "Config.hpp"

#define MAX_SEE_AHEAD 32.0f
#define MAX_AVOID_FORCE 1.0f
#define OBJ_RADIUS 20.0f

struct SteeringObject {
	EntityID entity;
	sf::Vector2f pos;
	sf::Vector2f velocity;
	float maxSpeed;
	float maxForce;
};

class Steering {
public:

	sf::Vector2f seek(SteeringObject currentObject, sf::Vector2f dpos) {
		sf::Vector2f steer = normalize(sf::Vector2f(dpos - currentObject.pos)) * currentObject.maxSpeed;
#ifdef STEERING_DEBUG
		if (steer != sf::Vector2f(0, 0))
			std::cout << "Steering: seek " << currentObject.entity << " " << steer << std::endl;
#endif
		steer -= currentObject.velocity;
		return limit(steer, currentObject.maxForce);
	}

	sf::Vector2f flee(SteeringObject currentObject, sf::Vector2f dpos) {
		sf::Vector2f steer = normalize(sf::Vector2f(currentObject.pos - dpos)) * currentObject.maxSpeed;
#ifdef STEERING_DEBUG
		if (steer != sf::Vector2f(0, 0))
			std::cout << "Steering: flee " << currentObject.entity << " " << steer << std::endl;
#endif
		return steer;
	}

	sf::Vector2f followFlowField(SteeringObject currentObject, sf::Vector2i direction) {
		sf::Vector2f steer = normalize(sf::Vector2f(direction)) * currentObject.maxSpeed;
		steer -= currentObject.velocity;
		return limit(steer, currentObject.maxForce);
	}

	sf::Vector2f followPath(SteeringObject currentObject, std::vector<sf::Vector2f> path, int idx) {
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
	sf::Vector2f avoid(SteeringObject currentObject, std::vector<sf::Vector2f> cases) {
		float separation = 24.0f;
		sf::Vector2f steer(0, 0);
		int count = 0;
		sf::Vector2f pos = currentObject.pos;
		for (sf::Vector2f &c : cases)
		{
			float d = length(currentObject.pos - c);
			if ((d > 0) && (d < separation)) {
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
		}

		return steer;
	}

	sf::Vector2f separate(SteeringObject currentObject, std::vector<SteeringObject> &others) {
		float desiredseparation = 32.0f;
		sf::Vector2f steer(0, 0);
		int count = 0;
		for (SteeringObject &other : others) {
			float d = length(currentObject.pos - other.pos);
			if ((d > 0) && (d < desiredseparation)) {
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

	sf::Vector2f cohesion(SteeringObject currentObject, std::vector<SteeringObject> &others) {
		float neighbordist = 32.0f;
		sf::Vector2f sum(0, 0);
		int count = 0;
		for (SteeringObject &other : others) {
			float d = length(currentObject.pos - other.pos);
			if ((d > 0) && (d < neighbordist)) {

				sum += other.pos;
				count++;
			}
		}
		if (count > 0) {
			sum /= (float)count;

			sum -= currentObject.velocity;
			sum = limit(sum, currentObject.maxForce);

			return seek(currentObject, sum);
		} else {
			return sf::Vector2f(0, 0);
		}
	}

	sf::Vector2f align (SteeringObject currentObject, std::vector<SteeringObject> &others) {
		float neighbordist = 32.0f;
		sf::Vector2f sum(0, 0);
		int count = 0;
		for (SteeringObject &other : others) {
			float d = length(currentObject.pos - other.pos);
			if ((d > 0) && (d < neighbordist)) {
				sum += other.velocity;
			}
		}
		if (count > 0) {
			sum /= (float)count;
			sum = normalize(sum);
			sum *= currentObject.maxSpeed;

			sum -= currentObject.velocity;
			sum = limit(sum, currentObject.maxForce);

			return sum;
		} else {
			return sf::Vector2f(0, 0);
		}
	}

	sf::Vector2f flock(SteeringObject currentObject, std::vector<SteeringObject> &others) {
		sf::Vector2f fv(0, 0);
		fv += this->separate(currentObject, others);
		fv += this->align(currentObject, others);
		fv += this->cohesion(currentObject, others);
		return fv;
	}


};