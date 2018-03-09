#pragma once

#include <SFML/Graphics.hpp>

#include "Entity.hpp"
#include "Options.hpp"

#include "Stages/GameStage.hpp"

struct AnimationFrameChanged {
	EntityID entity;
	std::string state;
	int frame;
};

struct StateChanged {
	EntityID entity;
	std::string state;
	unsigned int view;
	std::string newState;
};

struct EffectCreate {
	std::string name;
	EntityID emitterEnt; // emitter entity
	sf::Vector2f ppos;
	ParticleEffectOptions options;
};

struct EffectEnded {
	EntityID entity; // effect entity
};

struct EffectDestroy {
	EntityID entity; // effect entity
};

struct StageChange {
	NextStage nextStage;
};

struct EntityMustBeDestroyed {
	EntityID entity;
};

