#pragma once

#include "GameSystem.hpp"
#include "Events.hpp"

// buildings, units and resources deletion
class DeletionSystem : public GameSystem {
	std::queue<EntityID> entities;
	std::map<std::string,EntityID> corpses_and_ruins;
public:
	void init() override;
	void update(float dt) override;

// signals
void receive(const EntityDelete &event);

private:
	// init corpses and ruins tiles, must be called after player creation
	void initCorpses();
	void initCorpse(std::string name, EntityID playerEnt);
	void initRuin(std::string team, int i);

};
