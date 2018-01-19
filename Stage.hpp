#pragma once

//#include "Game.hpp"
class Game;

class Stage
{
public:
	Game* game;


	virtual void reset() = 0;
// pure virtual
	virtual void draw(const float dt) = 0;
	virtual void update(const float dt) = 0;
	virtual void handleEvent(sf::Event &event) = 0;

};
