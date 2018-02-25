#pragma once

#define MAX_SEE_AHEAD 32.0f
#define MAX_AVOID_FORCE 0.5f
#define OBJ_RADIUS 20.0f


struct SteeringObject {
	EntityID entity;
	sf::Vector2f pos;
	sf::Vector2f velocity;
};

class Steering {
public:
	std::vector<SteeringObject> objects;


	SteeringObject *findMostThreateningObject(SteeringObject currentObject) {

		SteeringObject *threatening = nullptr;
		float distance = std::numeric_limits<float>::max();

		for (auto &mo : objects) {
			float ndistance = vectorLength(mo.pos - currentObject.pos);

			sf::Vector2f ahead = currentObject.pos + vectorNormalize(currentObject.velocity - mo.velocity) * MAX_SEE_AHEAD;
			sf::Vector2f ahead2 = currentObject.pos + vectorNormalize(currentObject.velocity - mo.velocity) * MAX_SEE_AHEAD * 0.5f;

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
//		std::cout << "Steering: seek " << seekVel.x << "x" << seekVel.y << std::endl;
		return seekVel;
	}

	sf::Vector2f flee(SteeringObject currentObject, sf::Vector2f dpos, float speed) {
		sf::Vector2f fleeVel = vectorNormalize(sf::Vector2f(currentObject.pos - dpos)) * speed;
//		std::cout << "Steering: flee " << seekVel.x << "x" << seekVel.y << std::endl;
		return fleeVel;
	}

	sf::Vector2f collisionAvoidance(SteeringObject currentObject) {
		sf::Vector2f avoidance;

		SteeringObject *mostThreatening = this->findMostThreateningObject(currentObject);
		if (mostThreatening) {
			sf::Vector2f ahead = currentObject.pos + vectorNormalize(currentObject.velocity - mostThreatening->velocity) * MAX_SEE_AHEAD;
 			sf::Vector2f oCenterPos = mostThreatening->pos;

 			avoidance = ahead - mostThreatening->pos;

// 			float af = 1.0f;
 			sf::Vector2f vel = currentObject.velocity - mostThreatening->velocity;
 			float af = vectorLength(vel);
			avoidance = vectorNormalize(avoidance) * af * MAX_AVOID_FORCE;

			if(vectorLength(currentObject.velocity + avoidance) < af * MAX_AVOID_FORCE) { // take a perpendicular vector if forces avoid movement
				float x = avoidance.x;
				float y = avoidance.y;
				avoidance.x = -y;
				avoidance.y = x;
			}

		} else {
			avoidance = sf::Vector2f(0, 0);
		}

		return avoidance;
	}


};