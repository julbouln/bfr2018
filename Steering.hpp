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
};

class Steering {
public:
	std::vector<SteeringObject> objects;


	SteeringObject *findMostThreateningObject(SteeringObject currentObject) {
		SteeringObject *threatening = nullptr;
		float distance = std::numeric_limits<float>::max();

		for (auto &mo : objects) {
			float ndistance = vectorLength(mo.pos - currentObject.pos);

			sf::Vector2f velocity = currentObject.velocity - mo.velocity;

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

	sf::Vector2f seek(SteeringObject currentObject, sf::Vector2f dpos, float speed) {
		sf::Vector2f seekVel = vectorNormalize(sf::Vector2f(dpos - currentObject.pos)) * speed;
#ifdef STEERING_DEBUG
		if (seekVel != sf::Vector2f(0, 0))
			std::cout << "Steering: seek " << currentObject.entity << " " << seekVel.x << "x" << seekVel.y << std::endl;
#endif
		return seekVel;
	}

	sf::Vector2f flee(SteeringObject currentObject, sf::Vector2f dpos, float speed) {
		sf::Vector2f fleeVel = vectorNormalize(sf::Vector2f(currentObject.pos - dpos)) * speed;
#ifdef STEERING_DEBUG
		if (fleeVel != sf::Vector2f(0, 0))
			std::cout << "Steering: flee " << currentObject.entity << " " << fleeVel.x << "x" << fleeVel.y << std::endl;
#endif
		return fleeVel;
	}

	sf::Vector2f collisionAvoidance(SteeringObject currentObject) {
		sf::Vector2f avoidance;

		SteeringObject *mostThreatening = this->findMostThreateningObject(currentObject);
		if (mostThreatening) {
			sf::Vector2f velocity = currentObject.velocity - mostThreatening->velocity;
//			float dynLen = vectorLength(currentObject.velocity) / currentObject.maxSpeed;
			sf::Vector2f ahead = currentObject.pos + vectorNormalize(velocity) * MAX_SEE_AHEAD;

			avoidance = ahead - mostThreatening->pos;

// 			float af = 1.0f;
			float af = vectorLength(velocity);
//			float af = currentObject.maxSpeed;
			avoidance = vectorNormalize(avoidance) * af * MAX_AVOID_FORCE;

#ifdef STEERING_DEBUG
			std::cout << "Steering: avoid " << currentObject.entity << " threatened by " << mostThreatening->entity << " " << avoidance.x << "x" << avoidance.y << std::endl;
#endif

			if (vectorLength(currentObject.velocity + avoidance) < af * MAX_AVOID_FORCE) { // take a perpendicular vector if forces avoid movement

				float x = avoidance.x;
				float y = avoidance.y;
				avoidance.x = -y;
				avoidance.y = x;
#ifdef STEERING_DEBUG
				std::cout << "Steering: avoid " << currentObject.entity << " rotate by 90 " << mostThreatening->entity << " " << avoidance.x << "x" << avoidance.y << std::endl;
#endif

			}

		} else {
#ifdef STEERING_DEBUG
//		std::cout << "Steering: avoid " << currentObject.entity << " no threatening" << std::endl;
#endif
			avoidance = sf::Vector2f(0, 0);
		}

		return avoidance;
	}


};