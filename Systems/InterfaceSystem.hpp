#pragma once

#include <SFML/Graphics.hpp>

#include "third_party/imgui/imgui.h"
#include "third_party/imgui/imgui-sfml.h"
#include "third_party/imgui/imgui-sfml-extra.h"

#include "GameSystem.hpp"

class InterfaceSystem : public GameSystem {
public:
	sf::Sprite iface;
	sf::Sprite box;
	int box_w;
	sf::Sprite minimap_bg;
	int minimap_bg_h;
	sf::Sprite indice_bg;
	sf::Sprite indice;

	void init() override;
	void update(float dt) override;
	void draw(sf::RenderWindow &window, sf::IntRect clip, float dt);
	void debugGui(sf::RenderWindow &window, sf::View &view, int *gameSpeed, float dt);

	void addSelected(EntityID entity);
	void clearSelected() ;

private:
	void menuGui();
	void gameStateGui();
	void constructionProgressGui(EntityID consEnt);
	void actionGui();

};