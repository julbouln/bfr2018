#include "Helpers.hpp"
#include <cmath>

sf::Vector2f vectorNormalize(sf::Vector2f v) {
	sf::Vector2f nv;
	float length = vectorLength(v);
	if (length == 0) {
		nv.y = 0; nv.x = 0;
		length = 1;
	}
	nv.x = v.x / length;
	nv.y = v.y / length;
	return nv;
}

float vectorLength(sf::Vector2f v) {
	return sqrt(v.x * v.x + v.y * v.y);
}

float vectorLength(sf::Vector2i v) {
	return sqrt(v.x * v.x + v.y * v.y);
}

sf::Vector2f vectorRound(sf::Vector2f v) {
	sf::Vector2f rv = v;
	rv.x = round(rv.x);
	rv.y = round(rv.y);
	return rv;
}

float vectorDot(sf::Vector2f v1, sf::Vector2f v2) {
	return v1.x * v2.x + v1.y * v2.y;
}