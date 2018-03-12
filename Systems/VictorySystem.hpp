#pragma once

#include "GameSystem.hpp"

class VictorySystem : public GameSystem {
	bool scoreBonus;
	sf::Text scoreBonusText;
	sf::Sound scoreSound;

public:
	void init() override;
	void update(float dt) override;
	
	void draw(sf::RenderWindow &window, float dt);

	bool checkVictoryConditions(EntityID playerEnt);

private:
	void updatePlayerBonus(EntityID entity);
	void clearStats();
	void updateStats(float dt);
};