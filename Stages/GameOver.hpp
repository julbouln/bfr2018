#pragma once

#include "Game.hpp"
#include "GameStage.hpp"

class GameOver : public GameStage {
public:
	Player player;
	bool win;

	sf::Sprite background;

	GameOver(Game *game);

	void draw(float dt);
	void update(float dt);
	void handleEvent(sf::Event &event);
	void reset();
	void fadeOutCallback();
};