#pragma once

#include "Game.hpp"
#include "GameStage.hpp"
#include "GameEngine.hpp"

class MainMenu;

class PlayMenu : public GameStage {
public:
	int team;
	int mapSize;

	sf::Sprite background;
	PlayMenu(Game *game);

	void draw(float dt);
	void update(float dt);
	void handleEvent(sf::Event &event);
	void reset();
	void fadeOutCallback();
};