#pragma once

#include "Game.hpp"
#include "GameStage.hpp"
#include "GameEngine.hpp"

class GameOver : public GameStage {
public:
	Player player;
	bool win;

	sf::Sprite background;

	void draw(float dt) {
		background.setPosition(sf::Vector2f(0, 0));
		background.setScale(this->width / 800.0, this->height / 600.0);
		this->game->window.draw(background);

		this->guiPushStyles();

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.3f));
		ImGui::SetNextWindowPosCenter();
		if (ImGui::Begin("Game over", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
		{
			ImVec2 sz(192, 64);

			if (this->win) {
				ImGui::Text("Victory !");
			} else {
				ImGui::Text("Defeat !");
			}
			ImGui::Separator();

			ImGui::Text("Resources score: %d", player.resources);
			ImGui::Text("Kills: %d", player.stats["kills"]);
			ImGui::Text("Combo: %d", player.stats["combo"]);
			ImGui::Text("Serial killer: %d", player.stats["killer"]);
			ImGui::Text("Megakill: %d", player.stats["megakill"]);
			ImGui::Text("Barbarian: %d", player.stats["barbarian"]);
			ImGui::Text("Butchery: %d", player.stats["butchery"]);
			ImGui::Text("Combat score: %d", player.butchery);

			ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor(0, 0, 0, 255) );
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(16, 16));
			if (ImGui::Button("Play again", sz)) {
				nextStage = 1;
				this->fadeOut();
			}
			ImGui::PopStyleVar();
			ImGui::PopStyleColor();

			ImGui::End();
		}
		ImGui::PopStyleColor();

		this->guiPopStyles();

		ImGui::SFML::Render(this->game->window);
		this->drawVersion();

		this->updateFading();
	}

	void update(float dt) {
	}

	void handleEvent(sf::Event &event) {
	}

	GameOver(Game *game) {
		this->game = game;

		this->setSize(this->game->width, this->game->height);

		this->initEffects();
		this->fadeIn();

		background.setTexture(this->game->vault.factory.getTex("intro_background"));

	}

	void reset() {
		this->fadeIn();
		this->nextStage = 0;

//		GameEngine *engine = (GameEngine *)this->game->getStage("game");
//		delete engine;
//		this->game->unregisterStage("game");

	}

	void fadeOutCallback() {
		switch (nextStage) {
		case 1:
			this->game->pushRegisteredStage("play_menu");
			break;
		}

	}
};