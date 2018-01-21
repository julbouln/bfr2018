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
	void draw(float dt) {
		background.setPosition(sf::Vector2f(0, 0));
		background.setScale(this->width / 800.0, this->height / 600.0);
		this->game->window.draw(background);

		this->guiPushStyles();
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.3f));
		ImGui::SetNextWindowPosCenter();
		if (ImGui::Begin("Play menu", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
		{
			ImVec2 sz(128, 32);

			ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor(0, 0, 0, 255) );

			ImGui::Text("Team"); ImGui::SameLine();
			ImGui::RadioButton("Rebel", &team, 0); ImGui::SameLine();
			ImGui::RadioButton("Neonaz", &team, 1);

			const char* mapSizes[] = { "Small", "Medium", "Large" };
			ImGui::Text("Map size"); ImGui::SameLine();
			ImGui::Combo("", &mapSize, mapSizes, IM_ARRAYSIZE(mapSizes));   // Combo using proper array. You can also pass a callback to retrieve array value, no need to create/copy an array just for that.

//			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(16, 16));
			if (ImGui::Button("Back", sz)) {
				nextStage = 1;
				this->fadeOut();
			}; ImGui::SameLine();

			if (this->game->isRegisteredStage("game")) {
				if (ImGui::Button("Continue", sz)) {
					nextStage = 3;
					this->fadeOut();
				}; ImGui::SameLine();
			}

			if (ImGui::Button("Play", sz)) {
				nextStage = 2;
				this->fadeOut();
			}
//			ImGui::PopStyleVar();

			ImGui::PopStyleColor();

			ImGui::End();
		}
		ImGui::PopStyleColor();
		this->guiPopStyles();

		ImGui::SFML::Render(this->game->window);

		this->updateFading();
	}

	void update(float dt) {
	}

	void handleEvent(sf::Event &event) {
	}

	PlayMenu(Game *game) {
		this->game = game;

		this->setSize(this->game->width, this->game->height);

		this->initEffects();
		this->fadeIn();

		background.setTexture(this->game->vault.factory.getTex("intro_background"));

		this->team = 0;
		this->mapSize = 0;
	}

	void reset() {
		this->fadeIn();
		this->nextStage = 0;
	}

	void fadeOutCallback() {
		switch (nextStage) {
		case 1:
			this->game->pushRegisteredStage("main_menu");
			break;
		case 2: {
			std::string playerTeam = "rebel";
			unsigned int mapWidth = 64;
			unsigned int mapHeight = 64;

			switch (team) {
			case 0:
				playerTeam = "rebel";
				break;
			case 1:
				playerTeam = "neonaz";
				break;
			}

			switch (mapSize) {
			case 0:
				mapWidth = 48;
				mapHeight = 48;
				break;
			case 1:
				mapWidth = 64;
				mapHeight = 64;
				break;
			case 2:
				mapWidth = 128;
				mapHeight = 128;
				break;
			}

			if (this->game->isRegisteredStage("game")) {
				std::cout << "clear game"<<std::endl;
				GameEngine *engine = (GameEngine *)this->game->getStage("game");
				delete engine;
				this->game->unregisterStage("game");
			}

			this->game->registerStage("game", new GameEngine(this->game, mapWidth, mapHeight, playerTeam));
			this->game->pushRegisteredStage("game");
		}
		break;
		case 3:
			this->game->pushRegisteredStage("game");
			break;
		}

	}
};