#include "Helpers.hpp"
#include <cmath>


int getDirection(const sf::Vector2i &src, const sf::Vector2i &dst) {
	sf::Vector2i diff = dst - src;
	return getDirection(diff);
}

int getDirection(const sf::Vector2i &diff) {
	if (diff.x <= -1) {
		if (diff.y <= -1)
			return NorthWest;

		if (diff.y == 0)
			return West;

		if (diff.y >= 1)
			return SouthWest;
	}

	if (diff.x == 0) {
		if (diff.y <= -1)
			return North;

		if (diff.y == 0)
			return North;

		if (diff.y >= 1)
			return South;
	}

	if (diff.x >= 1) {
		if (diff.y <= -1)
			return NorthEast;

		if (diff.y == 0)
			return East;

		if (diff.y >= 1)
			return SouthEast;
	}

}