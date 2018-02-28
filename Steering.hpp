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
	std::vector<SteeringObject> objects;


	SteeringObject *findMostThreateningObject(SteeringObject currentObject) {
		SteeringObject *threatening = nullptr;
		float distance = std::numeric_limits<float>::max();

		for (auto &mo : objects) {
			float ndistance = vectorLength(mo.pos - currentObject.pos);

//			sf::Vector2f velocity = currentObject.velocity - mo.velocity;
			sf::Vector2f velocity = currentObject.velocity;

//			float dynLen = vectorLength(currentObject.velocity) / currentObject.maxSpeed;
//			sf::Vector2f ahead = currentObject.pos + vectorNormalize(velocity) * dynLen;
//			sf::Vector2f ahead2 = currentObject.pos + vectorNormalize(velocity) * dynLen * 0.5f;

			sf::Vector2f ahead = currentObject.pos + vectorNormalize(velocity) * MAX_SEE_AHEAD;
			sf::Vector2f ahead2 = currentObject.pos + vectorNormalize(velocity) * MAX_SEE_AHEAD * 0.5f;

			bool collision = vectorLength(mo.pos - ahead) <= OBJ_RADIUS || vectorLength(mo.pos - ahead2) <= OBJ_RADIUS || vectorLength(mo.pos - currentObject.pos) <= OBJ_RADIUS;
			if (collision && ndistance < distance) {
				distance = ndistance;
				threatening = &mo;
			}
		}
		return threatening;
	}

	sf::Vector2f limit(sf::Vector2f v, float max) {
		if (vectorSquare(v) > max * max) {
			return vectorNormalize(v) * max;
		} else {
			return v;
		}
	}

	sf::Vector2f seek(SteeringObject currentObject, sf::Vector2f dpos, float speed) {
		sf::Vector2f seekVel = vectorNormalize(sf::Vector2f(dpos - currentObject.pos)) * speed;
#ifdef STEERING_DEBUG
		if (seekVel != sf::Vector2f(0, 0))
			std::cout << "Steering: seek " << currentObject.entity << " " << seekVel.x << "x" << seekVel.y << std::endl;
#endif

		return this->limit(seekVel, currentObject.maxForce);
	}

	sf::Vector2f flee(SteeringObject currentObject, sf::Vector2f dpos, float speed) {
		sf::Vector2f fleeVel = vectorNormalize(sf::Vector2f(currentObject.pos - dpos)) * speed;
#ifdef STEERING_DEBUG
		if (fleeVel != sf::Vector2f(0, 0))
			std::cout << "Steering: flee " << currentObject.entity << " " << fleeVel.x << "x" << fleeVel.y << std::endl;
#endif
		return fleeVel;
	}


	sf::Vector2f followPath(SteeringObject currentObject, std::vector<sf::Vector2f> path, int idx) {
		int i = 0, index = 0;
		float dist = std::numeric_limits<float>::max();

		for (sf::Vector2f vect : path)
		{
			sf::Vector2f temp = (vect - currentObject.pos);
			float len = fabs(vectorLength(temp));
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

		return seek(currentObject, path[index], currentObject.maxSpeed);
	}

	sf::Vector2f avoid(SteeringObject currentObject, std::vector<sf::Vector2f> cases) {
		sf::Vector2f force(0, 0);
		sf::Vector2f pos = currentObject.pos;
		for (sf::Vector2f &c : cases)
		{
			force += vectorNormalize(sf::Vector2f(pos - c));
		}
		force = vectorNormalize(force) * currentObject.maxSpeed;
		force = this->limit(force, currentObject.maxForce);

		return force;
	}

	sf::Vector2f seperate(SteeringObject currentObject, std::vector<SteeringObject> &others) {
		float desiredseparation = 24.0f;
		sf::Vector2f sum(0, 0);
		int count = 0;
		for (SteeringObject &other : others) {
			float d = vectorLength(currentObject.pos - other.pos);
			if ((d > 0) && (d < desiredseparation)) {
				sf::Vector2f diff = vectorNormalize(currentObject.pos - other.pos) / d;

				sum += diff;
				count++;
			}
		}
		if (count > 0) {
			sum /= (float)count;
			sum = vectorNormalize(sum);
			sum *= currentObject.maxSpeed;
			sum = this->limit(sum, currentObject.maxForce);

			return sum;
		} else {
			return sf::Vector2f(0, 0);
		}
	}

	sf::Vector2f cohesion(SteeringObject currentObject, std::vector<SteeringObject> &others) {
		float neighbordist = 24.0f;
		sf::Vector2f sum(0, 0);
		int count = 0;
		for (SteeringObject &other : others) {
			float d = vectorLength(currentObject.pos - other.pos);
			if ((d > 0) && (d < neighbordist)) {

				sum += other.pos;
				count++;
			}
		}
		if (count > 0) {
			sum /= (float)count;
			sum = this->limit(sum, currentObject.maxForce);

			return seek(currentObject, sum, currentObject.maxSpeed);
		} else {
			return sf::Vector2f(0, 0);
		}
	}

	sf::Vector2f align (SteeringObject currentObject, std::vector<SteeringObject> &others) {
		float neighbordist = 24.0f;
		sf::Vector2f sum(0, 0);
		int count = 0;
		for (SteeringObject &other : others) {
			float d = vectorLength(currentObject.pos - other.pos);
			if ((d > 0) && (d < neighbordist)) {
				sum += other.velocity;
			}
		}
		if (count > 0) {
			sum /= (float)count;
			sum = vectorNormalize(sum);
			sum *= currentObject.maxSpeed;

			sum = this->limit(sum, currentObject.maxForce);

			return sum;
		} else {
			return sf::Vector2f(0, 0);
		}
	}

	sf::Vector2f flock(SteeringObject currentObject, std::vector<SteeringObject> &others) {
		sf::Vector2f fv(0, 0);
		fv += this->seperate(currentObject, others);
		fv += this->align(currentObject, others);
		fv += this->cohesion(currentObject, others);
		return fv;
	}



	sf::Vector2f collisionAvoidance(SteeringObject currentObject) {
		sf::Vector2f avoidance;

		SteeringObject *mostThreatening = this->findMostThreateningObject(currentObject);
		if (mostThreatening) {
//			sf::Vector2f velocity = currentObject.velocity - mostThreatening->velocity;
			sf::Vector2f velocity = currentObject.velocity;

//			float dynLen = vectorLength(currentObject.velocity) / currentObject.maxSpeed;
//			sf::Vector2f ahead = currentObject.pos + vectorNormalize(velocity) * dynLen;

			sf::Vector2f ahead = currentObject.pos + vectorNormalize(velocity) * MAX_SEE_AHEAD;

			avoidance = ahead - mostThreatening->pos;

// 			float af = 1.0f;
			float af = vectorLength(velocity);
//			float af = currentObject.maxSpeed;
			avoidance = vectorNormalize(avoidance) * af * MAX_AVOID_FORCE;

#ifdef STEERING_DEBUG
			std::cout << "Steering: avoid " << currentObject.entity << " threatened by " << mostThreatening->entity << " " << avoidance.x << "x" << avoidance.y << std::endl;
#endif

			/*
						if (vectorLength(currentObject.velocity + avoidance) < af * MAX_AVOID_FORCE) { // take a perpendicular vector if forces avoid movement

							float x = avoidance.x;
							float y = avoidance.y;
							avoidance.x = -y;
							avoidance.y = x;
			#ifdef STEERING_DEBUG
							std::cout << "Steering: avoid " << currentObject.entity << " rotate by 90 " << mostThreatening->entity << " " << avoidance.x << "x" << avoidance.y << std::endl;
			#endif

						}

						*/

		} else {
#ifdef STEERING_DEBUG
//		std::cout << "Steering: avoid " << currentObject.entity << " no threatening" << std::endl;
#endif
			avoidance = sf::Vector2f(0, 0);
		}

		return avoidance;
	}


};