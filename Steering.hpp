#pragma once

#include "Config.hpp"

#define MAX_SEE_AHEAD 32.0f
#define MAX_AVOID_FORCE 1.0f
#define OBJ_RADIUS 20.0f

#define MAX_QUEUE_AHEAD 16.0f
#define MAX_QUEUE_RADIUS 16.0f

#define AVOID_DIST 24.0f
#define SEPARATION_DIST 32.0f
#define COHESION_DIST 32.0f
#define ALIGN_DIST 32.0f

/*
struct SteeringObject {
	EntityID entity;
	sf::Vector2f pos;
	sf::Vector2f velocity;
	float maxSpeed;
	float maxForce;
};
*/

template <typename T>
class Steering {
public:

//	sf::Vector2f capRotation(const T &currentObject, const sf::Vector2f &desired, float maxAngle) {
//		float fullSteers = acos(dot(normalize(desired), normalize(currentObject.velocity))) / maxAngle;
//		return dot(leftperp(desired), currentObject.velocity);
//	}

	sf::Vector2f seek(const T &currentObject, const sf::Vector2f &dpos) {
		this->seek(currentObject, dpos, currentObject.maxSpeed);
	}

	sf::Vector2f seek(const T &currentObject, const sf::Vector2f &dpos, float speed) {
		sf::Vector2f steer = normalize(sf::Vector2f(dpos - currentObject.pos)) * speed;
#ifdef STEERING_DEBUG
		if (steer != sf::Vector2f(0, 0))
			std::cout << "Steering: seek " << currentObject.entity << " " << steer << std::endl;
#endif
		steer -= currentObject.velocity;
		return limit(steer, currentObject.maxForce);
	}

	sf::Vector2f arrive(const T &currentObject, const sf::Vector2f& target) {
		float range = 24.0f;
		sf::Vector2f steer = target - currentObject.pos;

		float d = distance(target, currentObject.pos);
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



	sf::Vector2f flee(const T &currentObject, const sf::Vector2f &dpos) {
		sf::Vector2f steer = normalize(sf::Vector2f(currentObject.pos - dpos)) * currentObject.maxSpeed;
#ifdef STEERING_DEBUG
		if (steer != sf::Vector2f(0, 0))
			std::cout << "Steering: flee " << currentObject.entity << " " << steer << std::endl;
#endif
		return steer;
	}

	sf::Vector2f followFlowField(const T &currentObject, const sf::Vector2i &direction) {
		sf::Vector2f steer = normalize(sf::Vector2f(direction)) * currentObject.maxSpeed;
		steer -= currentObject.velocity;

#ifdef STEERING_DEBUG
		if (steer != sf::Vector2f(0, 0))
			std::cout << "Steering: followFlowField " << currentObject.entity << " " << steer << std::endl;
#endif

		return limit(steer, currentObject.maxForce);
	}

	sf::Vector2f followPath(T &currentObject, std::vector<sf::Vector2f> path, int idx) {
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

//	// same than separate ?
	sf::Vector2f avoid(const T &currentObject, std::vector<sf::Vector2f> &cases) {
		sf::Vector2f steer(0, 0);
		int count = 0;
		sf::Vector2f pos = currentObject.pos;
		for (sf::Vector2f &c : cases)
		{
			float d = distance(currentObject.pos, c);
			if (d <= AVOID_DIST) {
				steer += normalize(currentObject.pos - c);
				count++;
			}
		}
		if (count > 0) {
			steer /= (float)count;
			steer = normalize(steer);
			steer *= currentObject.maxSpeed;

			steer -= currentObject.velocity;
			steer = limit(steer, currentObject.maxForce);

#ifdef STEERING_DEBUG
			if (steer != sf::Vector2f(0, 0))
				std::cout << "Steering: avoid " << currentObject.entity << " " << steer << std::endl;
#endif

			return steer;
		} else {
			return sf::Vector2f(0, 0);
		}
	}

	sf::Vector2f separate(const T &currentObject, std::vector<T> &others) {
		sf::Vector2f steer(0, 0);
		int count = 0;
		for (auto &other : others) {
			float d = distance(currentObject.pos, other.pos);
			if (d > 0.0f && d < SEPARATION_DIST) {
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

#ifdef STEERING_DEBUG
			if (steer != sf::Vector2f(0, 0))
				std::cout << "Steering: separate " << currentObject.entity << " " << steer << std::endl;
#endif

			return steer;
		} else {
			return sf::Vector2f(0, 0);
		}
	}

	sf::Vector2f cohesion(const T &currentObject, std::vector<T> &others) {
		sf::Vector2f steer(0, 0);
		int count = 0;
		for (T &other : others) {
			float d = distance(currentObject.pos, other.pos);
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

	sf::Vector2f align (const T &currentObject, std::vector<T> &others) {
		sf::Vector2f steer(0, 0);
		int count = 0;
		for (auto &other : others) {
			float d = distance(currentObject.pos, other.pos);
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

	sf::Vector2f flock(const T &currentObject, std::vector<T> &others) {
		sf::Vector2f fv(0, 0);
		fv += this->separate(currentObject, others);
		fv += this->align(currentObject, others);
		fv += this->cohesion(currentObject, others);
		return fv;
	}

	T* getNeighborAhead(const T &currentObject, std::vector<T> &neighbors) {
		T *ret = nullptr;
		sf::Vector2f qa = normalize(currentObject.velocity) * MAX_QUEUE_AHEAD;

		sf::Vector2f ahead = currentObject.pos + qa;

		for (auto &neighbor : neighbors) {
			float d = distance(ahead, neighbor.pos);

			if (d <= MAX_QUEUE_RADIUS) {
				ret = &neighbor;
				break;
			}
		}

		return ret;
	}

	sf::Vector2f queue(const T &currentObject, std::vector<T> &neighbors) {
		T *neighbor = this->getNeighborAhead(currentObject, neighbors);

		if (neighbor) {
			sf::Vector2f steer = (currentObject.velocity * 0.3f);

			steer -= currentObject.velocity;
			steer = limit(steer, currentObject.maxForce);

			return steer;
		}

		return sf::Vector2f(0, 0);

	}

	sf::Vector2f queue(const T &currentObject, std::vector<T> &neighbors, sf::Vector2f currentSteer) {
		sf::Vector2f v = currentObject.velocity;
		sf::Vector2f brake;

		T *neighbor = this->getNeighborAhead(currentObject, neighbors);

		if (neighbor) {
			brake.x = -currentSteer.x * 0.8f;
			brake.y = -currentSteer.y * 0.8f;
			v *= -1.0f;
			brake += v;

			if (distance(currentObject.pos, neighbor->pos) <= MAX_QUEUE_RADIUS) {
				sf::Vector2f steer = (currentObject.velocity * 0.3f);

				steer -= currentObject.velocity;
				steer = limit(steer, currentObject.maxForce);

				return steer;

			}
		}

		return brake;
	}
};