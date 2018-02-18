#pragma once

#include "Game.hpp"
#include "GameStage.hpp"
#include "PlayMenu.hpp"

class MainMenu : public GameStage {
public:
	sf::Sprite background;

	MainMenu(Game *game);

	void draw(float dt);
	void update(float dt);
	void handleEvent(sf::Event &event);
	void reset();
	void fadeOutCallback();
};