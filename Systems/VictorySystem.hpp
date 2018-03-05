#pragma once

#include "GameSystem.hpp"

class VictorySystem : public GameSystem {
	bool scoreBonus;
	sf::Text scoreBonusText;
	sf::Sound scoreSound;

public:
	void init();

	void draw(sf::RenderWindow &window, float dt);
	void updatePlayerBonus(EntityID entity);

	void clearStats();
	void updateStats(float dt);
	float resourcesVictory();
	float butcheryVictory();
	bool checkVictoryConditions(EntityID playerEnt);

};