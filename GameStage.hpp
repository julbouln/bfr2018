#pragma once

#include "Stage.hpp"
#include "Game.hpp"


class GameStage : public Stage {
public:

	unsigned int width;
	unsigned int height;

	sf::RectangleShape fade;
	int fadeStep;
	int fadeType;
	int fadeSpeed;

	int nextStage;

	void setSize(unsigned int width, unsigned int height) {
		this->width = width;
		this->height = height;
	}

	void fadeOut() {
		this->fadeStep = 0;
		this->fadeType = 1;
	}

	void fadeIn() {
		this->fadeStep = 255;
		this->fadeType = 0;
	}

	void updateFading() {
		if (fadeType == 1 && fadeStep < 255)
		{
			fadeStep += fadeSpeed;
			if (fadeStep > 255)
				fadeStep = 255;
			fade.setFillColor(sf::Color(0, 0, 0, fadeStep));
			this->game->window.draw(fade);
		}

		if (fadeType == 0 && fadeStep > 0)
		{
			fadeStep -= fadeSpeed;
			if (fadeStep < 0)
				fadeStep = 0;
			fade.setFillColor(sf::Color(0, 0, 0, fadeStep));
			this->game->window.draw(fade);
		}

		if(fadeType == 1 && fadeStep >= 255) {
			fadeOutCallback();
		}
	}

	void initEffects() {
		fade.setPosition(sf::Vector2f(0, 0));
		fade.setFillColor(sf::Color(0, 0, 0, 255));
		fade.setSize(sf::Vector2f(this->width, this->height));
		fadeSpeed = 5;
	}

	virtual void fadeOutCallback() = 0;

};