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
			sf::Vector2f oCenterPos = mo.pos;
//			mo.pos.x+=16.0f;
//			mo.pos.y+=16.0f;

			float ndistance = vectorLength(oCenterPos - currentObject.pos);

			sf::Vector2f ahead = currentObject.pos + vectorNormalize(currentObject.velocity - mo.velocity) * MAX_SEE_AHEAD;
			sf::Vector2f ahead2 = currentObject.pos + vectorNormalize(currentObject.velocity - mo.velocity) * MAX_SEE_AHEAD * 0.5f;

			bool collision = vectorLength(oCenterPos - ahead) <= OBJ_RADIUS || vectorLength(oCenterPos - ahead2) <= OBJ_RADIUS || vectorLength(oCenterPos - currentObject.pos) <= OBJ_RADIUS;
			if (collision && ndistance < distance) {
					distance = ndistance;
					threatening = &mo;
			}
		}
		return threatening;
	}

	sf::Vector2f seek(SteeringObject currentObject, sf::Vector2f dpos, float speed) {
		sf::Vector2f seekVel = vectorNormalize(sf::Vector2f(dpos - currentObject.pos)) * speed;
//		std::cout << "FlowFieldPath: seek " << seekVel.x << "x" << seekVel.y << std::endl;
		return seekVel;
	}

	sf::Vector2f collisionAvoidance(SteeringObject currentObject) {
		sf::Vector2f avoidance;

		SteeringObject *mostThreatening = this->findMostThreateningObject(currentObject);
		if (mostThreatening) {
			sf::Vector2f ahead = currentObject.pos + vectorNormalize(currentObject.velocity - mostThreatening->velocity) * MAX_SEE_AHEAD;
 			sf::Vector2f oCenterPos = mostThreatening->pos;
 //			oCenterPos.x+=16.0f;
// 			oCenterPos.y+=16.0f;

// 			avoidance = currentObject.pos - oCenterPos;

 			avoidance = ahead - oCenterPos;
 //			avoidance.x += 0.1f;
// 			avoidance.y += 0.1f;

// 			float f=fabs(vectorDot(currentObject.velocity, oCenterPos - currentObject.pos));
// 			float af = 1.0f;
 			sf::Vector2f vel = currentObject.velocity - mostThreatening->velocity;
 			float af = vectorLength(vel);
			avoidance = vectorNormalize(avoidance) * af * MAX_AVOID_FORCE;

			if(vectorLength(currentObject.velocity + avoidance) < af * MAX_AVOID_FORCE) {
				float x = avoidance.x;
				float y = avoidance.y;
				avoidance.x = -y;
				avoidance.y = x;
			}
/*
						if (avoidance.x == 0) {
							avoidance.x = avoidance.y;
						} else if (avoidance.y == 0) {
							avoidance.y = avoidance.x;
						} else if (avoidance.x != 0 && abs(avoidance.x) == abs(avoidance.y)) {
							avoidance.x = -avoidance.x;
						}
*/

		} else {
			avoidance = sf::Vector2f(0, 0);
		}

		return avoidance;
	}


};