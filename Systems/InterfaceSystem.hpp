#pragma once

#include <SFML/Graphics.hpp>

#include "third_party/imgui/imgui.h"
#include "third_party/imgui/imgui-sfml.h"
#include "third_party/imgui/imgui-sfml-extra.h"

#include "GameSystem.hpp"

class InterfaceSystem : public GameSystem {
public:
	void init();
	void update(float dt);
	void draw(sf::RenderWindow &window, sf::IntRect clip, float dt);

	inline float scaleX() const {
		return this->screenWidth / 800.0;
	}
	inline float scaleY() const {
		return this->screenHeight / 600.0;
	}

	void clearSelection() ;
	void updateSelected(float dt);
	void orderSelected(sf::Vector2f destpos);

	void menuGui();
	void gameStateGui();
	void constructionProgressGui(EntityID consEnt);
	void actionGui();

};