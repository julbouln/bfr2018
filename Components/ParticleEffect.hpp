#pragma once

#include "third_party/Particles/ParticleSystem.h"

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
