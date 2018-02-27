#pragma once
#include <SFML/Graphics.hpp>

struct CompareVector2i
{
	bool operator()(sf::Vector2i a, sf::Vector2i b) const
	{
		if (a.x < b.x)
			return true;
		else if (b.x < a.x)
			return false;
		else
			return a.y < b.y;
	}
};

sf::Vector2f vectorNormalize(sf::Vector2f v);
float vectorLength(sf::Vector2f v);
float vectorLength(sf::Vector2i v);
float vectorSquare(sf::Vector2f v);
sf::Vector2f vectorRound(sf::Vector2f v);
sf::Vector2f vectorTrunc(sf::Vector2f v);
float vectorDot(sf::Vector2f v1, sf::Vector2f v2);
