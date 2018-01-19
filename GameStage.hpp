#pragma once

#include "Stage.hpp"
#include "Game.hpp"


class GameStage : public Stage {
public:

	unsigned int width;
	unsigned int height;

	sf::RectangleShape fade;
	int fadeStep;

	void setSize(unsigned int width, unsigned int height) {
		this->width = width;
		this->height = height;
	}

	void fadeOut() {
		this->fadeStep = 255;
	}

	void updateFadeOut() {
		if (fadeStep > 0)
		{
			fadeStep -= 3;
			if (fadeStep < 0)
				fadeStep = 0;
			fade.setFillColor(sf::Color(0, 0, 0, fadeStep));
			this->game->window.draw(fade);
		}
	}

	void fadeIn() {
		this->fadeStep = 0;
	}

	void updateFadeIn() {
		if (fadeStep < 255)
		{
			fadeStep += 3;
			if (fadeStep > 255)
				fadeStep = 255;
			fade.setFillColor(sf::Color(0, 0, 0, fadeStep));
			this->game->window.draw(fade);
		}
	}

	void initEffects() {
		fade.setPosition(sf::Vector2f(0, 0));
		fade.setFillColor(sf::Color(0, 0, 0, 255));
		fade.setSize(sf::Vector2f(this->width, this->height));
	}

};