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
	sf::Vector2f(0.0, -1.0),
	sf::Vector2f(1.0, -1.0),
	sf::Vector2f(1.0, 0.0),
	sf::Vector2f(1.0, 1.0),
	sf::Vector2f(0.0, 1.0),
	sf::Vector2f(-1.0, -1.0),
	sf::Vector2f(-1.0, 0.0),
	sf::Vector2f(-1.0, 1.0)
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

struct Timer {
	EntityID emitterEntity;
	std::string name;
	bool loop; // is looping
	float duration; // loop duration
	float t; // current time since beginning
	int l; // number of loop since beginning

	Timer(EntityID entity, std::string n, float d, bool lo) {
		this->emitterEntity = entity;
		this->name = n;
		this->t = 0.0;
		this->l = 0;		
		this->duration = d;
		this->loop = lo;
	}


	Timer(std::string n, float d, bool lo) {
		this->emitterEntity = 0;
		this->name = n;
		this->t = 0.0;
		this->l = 0;		
		this->duration = d;
		this->loop = lo;
	}

	Timer() {
		this->emitterEntity = 0;
		this->loop = true;		
		this->t = 0.0;
		this->l = 0;		
	}

	inline bool ended() const {
		return !this->loop && this->l == 1;
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
//	float t;
	// number of loop since animation started
//	int l;

	AnimatedSpriteView() {
		this->loop = true;
//		this->l = 0;
//		this->t = 0.0;
		this->currentFrame = 0;
	}
};

struct StaticSpritesheet {
	std::map<std::string, std::vector<SpriteView>> states;
};

struct AnimatedSpritesheet {
	std::map<std::string, std::vector<AnimatedSpriteView>> states;
};

// unit or building
struct GameObject {
	std::string name;
	std::string team;

	float life;
	float maxLife;
	unsigned int view;

	bool mapped;

	EntityID player;
};

struct Attack {
	unsigned int power;
	unsigned int distance;
	unsigned int maxDistance;
};

enum class Attitude {
	Aggressive,
	Pacifist
};

enum class GroupFormation {
	Square,
	TwoLine,
	OneLine
};

typedef entt::HashedString::hash_type SpecialSkill;
typedef entt::HashedString SpecialSkillStr;

enum class TargetType {
	None,
	Attack,
	Bomb,
	Special
};

struct Unit {
	unsigned int cost;
	float speed;
	Attack attack1;
	Attack attack2;

	TargetType targetType;
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

	SpecialSkill special;

	Unit() {
		this->averageCount = 1;
		this->velocity = sf::Vector2f(0, 0);
		this->averageVelocity = sf::Vector2f(0, 0);
		this->direction = sf::Vector2i(0, 0);
		this->nopath = 0;
		this->reallyNopath = 0;
		this->commanded = false;
		this->pathUpdate = false;
		this->special = SpecialSkillStr("none");

		this->targetType = TargetType::None;
		this->targetEnt = 0;
	}
};

#define MAX_FORCE 0.1f

class PathfindingObject : public sf::Vector2f {
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
		this->x = tile->ppos.x;
		this->y = tile->ppos.y;

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

enum class Action {
	None,
	Select,
	Build,
	Attack,
	Move,
	Special
};

enum class MoveView {
	DontMove,
	MoveWest,
	MoveEast,
	MoveNorth,
	MoveSouth
};

struct GameController {
	Action action;
	MoveView moveView;

	sf::Vector2f selectionStart;
	sf::Vector2f selectionEnd;

	std::vector<EntityID> selectedObjs;

	EntityID currentBuild;
	std::string currentBuildType;

	EntityID currentPlayer;

	int debugCorner;
	EntityID selectedDebugObj;
	bool showDebugWindow;

	GameController() {
		this->action = Action::None;
		this->moveView = MoveView::DontMove;
		this->currentPlayer = 0;
		this->currentBuild = 0;

		this->selectedDebugObj = 0;
		this->debugCorner = 1;
		this->showDebugWindow = false;
	}
};