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

struct EffectEnded {
	EntityID entity; // effect entity
};

struct EffectCreate {
	std::string name;
	EntityID emitterEnt; // emitter entity
	sf::Vector2f ppos;
	ParticleEffectOptions options;
};

struct EffectDestroy {
	EntityID entity; // effect entity
};

struct GameStageChange {
	NextStage nextStage;
};

struct EntityDelete {
	EntityID entity;
};

struct SoundPlay {
	std::string name;
	int priority;
	bool relative;
	sf::Vector2i pos;
};


