#pragma once

#include <set>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include "Entity.hpp"

#include "Map.hpp"
#include "BrainTree/BrainTree.h"

#include "Options.hpp"

#include "FlowField.hpp"

#include "ParticleEffect.hpp"

static const sf::Vector2f directionVectors[8] = {
	sf::Vector2f(0.0,-1.0),
	sf::Vector2f(1.0,-1.0),
	sf::Vector2f(1.0,0.0),
	sf::Vector2f(1.0,1.0),
	sf::Vector2f(0.0,1.0),
	sf::Vector2f(-1.0,-1.0),
	sf::Vector2f(-1.0,0.0),
	sf::Vector2f(-1.0,1.0)
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

	Tile() {
		this->z = 0;
		this->view = 0;
		this->state = "idle";
		this->shader = false;
	}
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

	AnimatedSpriteView() {
		this->loop = true;
		this->l = 0;
		this->t = 0.0;
		this->currentFrame = 0;
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
	unsigned int maxDistance;	
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
	unsigned int cost;
	float speed;
	Attack attack1;
	Attack attack2;

	EntityID targetEnt;
	sf::Vector2i targetPos;

	sf::Vector2i destpos;
	sf::Vector2f velocity;

	sf::Vector2f averageVelocity;
	int averageCount;

	sf::Vector2i direction;

	sf::Vector2i pathPos; // map pos
	bool pathUpdate;
	bool commanded;

	unsigned int nopath;
	unsigned int reallyNopath;

	std::string sound;
	std::map<std::string, int> soundActions;

	std::string attackSound;

	FlowFieldPath flowFieldPath;

	bool canDestroyResources;

	Unit() {
		this->averageCount = 1;
		this->velocity = sf::Vector2f(0,0);
		this->averageVelocity = sf::Vector2f(0,0);
		this->direction = sf::Vector2i(0,0);
		this->nopath = 0;
		this->reallyNopath = 0;
		this->canDestroyResources = false;
		this->commanded = false;
		this->pathUpdate = false;
	}
};

#define MAX_FORCE 0.2f

class PathfindingObject : public sf::FloatRect {
public:
	EntityID entity;
	sf::Vector2f pos;
	sf::Vector2f velocity;
	float maxSpeed;
	float maxForce;

	// keep the components pointers for update
    PathfindingObject(EntityID ent, Tile &_tile, Unit &_unit) : entity(ent), tile(&_tile), unit(&_unit) {
    	this->maxForce = MAX_FORCE;
    	this->update();
    }

    void update() {
    	this->left = tile->ppos.x - 16.0f;
    	this->top = tile->ppos.y - 16.0f;
    	this->width = 32.0f;
    	this->height = 32.0f;

    	this->pos = tile->ppos;
    	this->velocity = unit->velocity;
    	this->maxSpeed = unit->speed;
    }
private:
	Unit *unit;
	Tile *tile;

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
	unsigned int maxLevel;
	float grow;
	float maxGrow;
	float growRate;

	Resource() {
		this->level = 1;
		this->maxLevel = 3;
		this->grow = 0.0;
		this->maxGrow = 10.0;
		this->growRate = 0.1;
	}
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

