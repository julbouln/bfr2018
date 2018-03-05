#pragma once

#include "GameSystem.hpp"

#include "third_party/SimplexNoise.h"

#define ALT_TILES 3

class GameGeneratorSystem : public GameSystem {
public:
	EntityID generate(int mapWidth, int mapHeight, std::string playerTeam);

private:
	void generateMap(unsigned int width, unsigned int height);
	std::vector<int> generateColorIndices();
	sf::Vector2i getInitialPosition(sf::Vector2i pos);
	std::vector<sf::Vector2i> generateInitialPositions();
};
