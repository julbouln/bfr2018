#pragma once

#include <set>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include "Entity.hpp"

#include "Map.hpp"
#include "BrainTree/BrainTree.h"

#include "Particles/ParticleSystem.h"

#include "ShaderOptions.hpp"

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

struct Tile {
	sf::Vector2f psize; // pixel size
	sf::Vector2i size; // map size

	sf::Vector2f ppos; // pixel pos
	sf::Vector2i pos; // map pos

	sf::Vector2i offset; // offset

	int z;

	sf::IntRect centerRect;

	sf::Sprite sprite;

	std::string state;
	unsigned int view;

	bool shader;
	std::string shaderName;
	ShaderOptions shaderOptions;
};

struct SpriteView {
	sf::Vector2i currentPosition;
};

struct AnimatedSpriteView {
	// is looping
	bool loop;
	// each frame duration
	float duration;
	// frames
	std::vector<sf::Vector2i> frames;
	// current frame
	int currentFrame;
	// current time since the animation loop started
	float t;
	// number of loop since animation started
	int l;

	std::function<void(int)> frameChangeCallback;

	AnimatedSpriteView() {
		this->l = 0;
		this->t = 0.0;
		this->loop = true;
		this->currentFrame = 0;
		this->frameChangeCallback = [](int frame) {};
	}
};

struct StaticSpritesheet {
	std::map<std::string, std::vector<SpriteView>> states;
};

struct AnimatedSpritesheet {
	std::map<std::string, std::vector<AnimatedSpriteView>> states;
};

struct Attack {
	unsigned int power;
	unsigned int distance;
};

struct ParticleEffect {
	particles::ParticleSystem *particleSystem;
	particles::ParticleSpawner *spawner;
	float lifetime;
	float currentTime;
	int particles;
	bool continuous;
};

struct Effects {
	std::map<std::string, std::string> effects;
};

// unit or building
struct GameObject {
	std::string name;
	std::string team;

	float life;
	float maxLife;
	unsigned int view;

	bool mapped;
	bool destroy;

	EntityID player;
};

enum class GroupFormation {
	Square,
	TwoLine,
	OneLine
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

	std::string sound;
	std::map<std::string, int> soundActions;

	std::string attackSound;
};

struct Building {
	float buildTime;
	float maxBuildTime;
	EntityID construction;
	EntityID constructedBy;
};

struct Resource {
	std::string type;
	unsigned int level;
	float grow;
};

struct Decor {
	bool blocking;
};

struct FrontPoint {
	sf::Vector2i pos;
	int priority;
};


class FrontPointCompare
{
public:
	bool operator() (FrontPoint &l, FrontPoint &r)
	{
		return l.priority < r.priority;
	}
};


struct Player {
	std::string team;
	int colorIdx;
	sf::Color color;
	bool ai;
	std::string resourceType;
	int resources;

	std::set<EntityID> kills;

	int butchery;

	sf::Vector2i initialPos;
	bool enemyFound;
	sf::Vector2i enemyPos;

	std::vector<sf::Vector2i> allFrontPoints;
	std::vector<FrontPoint> frontPoints;

	Fog fog;

	std::map<std::string, std::vector<EntityID>> objsByType;

	BrainTree::BehaviorTree aiTree;

	EntityID rootConstruction;

	// stats
	std::map<std::string, int> stats;
};

