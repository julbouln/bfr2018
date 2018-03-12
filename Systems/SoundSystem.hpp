#pragma once

#include "Events.hpp"
#include "GameSystem.hpp"

class SoundPlayCompare
{
public:
	bool operator() (SoundPlay &l, SoundPlay &r)
	{
		return l.priority < r.priority;
	}
};

class SoundSystem : public GameSystem {
	std::priority_queue<SoundPlay, std::vector<SoundPlay>, SoundPlayCompare> sounds;
	std::list<sf::Sound> playing;
public:
	void init() override;
	void update(float dt) override;

	// signals
	void receive(const SoundPlay &event);
};