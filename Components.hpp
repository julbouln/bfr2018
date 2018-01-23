#pragma once

#include <set>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include "AnimationHandler.hpp"
#include "Entity.hpp"

#include "Map.hpp"
#include "BrainTree/BrainTree.h"

struct Tile {
	sf::Vector2f psize; // pixel size
	sf::Vector2i size; // map size

	sf::Vector2f ppos; // pixel pos
	sf::Vector2i pos; // map pos

	sf::Vector2i offset; // offset

	int z;

	sf::IntRect centerRect;

	sf::Sprite sprite;
	std::map<std::string,AnimationHandler> animHandlers;

    std::string state;
    unsigned int direction;

};

struct Attack {
	unsigned int power;
	unsigned int distance;
};


struct MapEffect {
	bool show;
	float speed;
	std::vector<sf::Vector2f>positions;
	int curPosition;
	sf::Sound sound;
};

// unit or building
struct GameObject {
	float life;
	float maxLife;
	unsigned int view;
	bool mapped;
	bool destroy;

	std::string name;
	std::string team;
	EntityID player;
	std::map<std::string, EntityID> effects;
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
	unsigned int speed;
	Attack attack1;
	Attack attack2;

	EntityID destAttack;
	sf::Vector2i destAttackPos;

	sf::Vector2i destpos;
	sf::Vector2i nextpos;
	unsigned int nopath;

	sf::Sound sound;
	std::map<std::string, int> soundActions;

	sf::Sound attackSound;
};

struct Building {
	float buildTime;
	float maxBuildTime;
	EntityID construction;
	EntityID constructedBy;
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
	int colorIdx;
	bool ai;
	ResourceType resourceType;
	int resources;

	std::set<EntityID> kills;

	int butchery;

	sf::Vector2i initialPos;
	bool enemyFound;
	sf::Vector2i enemyPos;

	Fog fog;

	std::map<std::string, std::vector<EntityID>> objsByType;

	BrainTree::BehaviorTree aiTree;

	EntityID rootConstruction;

	// stats
	std::map<std::string, int> stats;
};