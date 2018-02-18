#pragma once

#include "third_party/Particles/ParticleSystem.h"

struct ParticleEffect {
	particles::ParticleSystem *particleSystem;
	particles::ParticleSpawner *spawner;
	float lifetime;
	float currentTime;
	int particles;
	bool continuous;
	bool alwaysVisible;
	sf::Vector2f pos;
	sf::Vector2f destpos;

	std::function<void(void)> effectEndCallback;

	ParticleEffect() {
		this->particleSystem = nullptr;
		this->spawner = nullptr;
		this->effectEndCallback = []() {};
	}
};

struct Effects {
	std::map<std::string, std::string> effects;
};
