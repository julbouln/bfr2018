#pragma once

#include <SFML/Graphics.hpp>

#include "AnimationHandler.hpp"
#include "Entity.hpp"

struct Tile {
	sf::Vector2f psize; // pixel size
	sf::Vector2i size; // map size

	sf::Vector2f ppos; // pixel pos
	sf::Vector2i pos; // map pos

	sf::Vector2i offset; // offset

	sf::Sprite sprite;
	std::map<std::string,AnimationHandler> animHandlers;

    int tileVariant;
    std::string state;
    unsigned int direction;

};

struct Attack {
	unsigned int power;
	unsigned int distance;
};

struct GameObject {
	unsigned int life;
	unsigned int view;

	std::string name;
	std::string team;
	EntityID player;
};

enum {
	North,
	NorthEast,
	East,
	SouthEast,
	South,
	NorthWest,
	West,
	SouthWest
};

struct Unit {
	unsigned int direction;
	unsigned int speed;
	Attack attack1;
	Attack attack2;

	sf::Vector2i destpos;
	sf::Vector2i nextpos;
};

struct Building {
	unsigned int buildTime;
	bool built;
};

enum class ResourceType {
	Nature,
	Pollution
};

struct Resource {
	ResourceType type;
	unsigned int level;
	float grow;
};

struct Player {
	std::string team;
	bool ai;
	ResourceType resourceType;
	int resources;
	int butchery;
};