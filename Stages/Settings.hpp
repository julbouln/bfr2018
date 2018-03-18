#pragma once

#include "Game.hpp"
#include "GameStage.hpp"
#include "PlayMenu.hpp"

class Settings : public GameStage {
public:
	sf::Sprite background;
	int screenSize;
	bool fullscreen;

	Settings(Game *game);

	void draw(float dt);
	void update(float dt);
	void handleEvent(sf::Event &event);
	void reset();
	void fadeOutCallback();
};