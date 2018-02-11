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
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		ImGui::SetNextWindowPosCenter();
		if (ImGui::Begin("Play menu", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
		{
			ImVec2 sz(128, 32);

			ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor(0, 0, 0, 255) );

			ImGui::Text("Team"); ImGui::SameLine();
			ImGui::RadioButton("Rebel", &team, 0); ImGui::SameLine();
			ImGui::RadioButton("Neonaz", &team, 1); ImGui::SameLine();
			ImGui::RadioButton("Spectator", &team, 2);

			const char* mapSizes[] = { "Tiny", "Small", "Medium", "Large", "Huge"};
			ImGui::Text("Map size"); ImGui::SameLine();
			ImGui::Combo("", &mapSize, mapSizes, IM_ARRAYSIZE(mapSizes));   // Combo using proper array. You can also pass a callback to retrieve array value, no need to create/copy an array just for that.

//			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(16, 16));
			if (ImGui::Button("Back", sz)) {
				nextStage = NextStageStr("main_menu");
				this->fadeOut();
			}; ImGui::SameLine();

			if (this->game->isRegisteredStage("game")) {
				if (ImGui::Button("Continue", sz)) {
					nextStage = NextStageStr("continue_game");
					this->fadeOut();
				}; ImGui::SameLine();
			}

			if (ImGui::Button("Play", sz)) {
				nextStage = NextStageStr("new_game");
				this->fadeOut();
			}
//			ImGui::PopStyleVar();

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

	PlayMenu(Game *game) {
		this->game = game;

		this->setSize(this->game->width, this->game->height);

		this->initEffects();
		this->fadeIn();

		background.setTexture(this->game->vault.factory.getTex("intro_background"));

		this->team = 0;
		this->mapSize = 2;
	}

	void reset() {
		this->fadeIn();
		this->nextStage = 0;
	}

	void fadeOutCallback() {
		switch (nextStage) {
		case NextStageStr("main_menu"):
			this->game->pushRegisteredStage("main_menu");
			break;
		case NextStageStr("new_game"): {
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
			case 2:
				playerTeam = "neutral";
				break;
			}

			switch (mapSize) {
			case 0:
				mapWidth = 48;
				mapHeight = 48;
				break;
			case 1:
				mapWidth = 72;
				mapHeight = 72;
				break;
			case 2:
				mapWidth = 96;
				mapHeight = 96;
				break;
			case 3:
				mapWidth = 120;
				mapHeight = 120;
				break;
			case 4:
				mapWidth = 144;
				mapHeight = 144;
				break;
			}

			if (this->game->isRegisteredStage("game")) {
				GameEngine *engine = (GameEngine *)this->game->getStage("game");
				delete engine;
				this->game->unregisterStage("game");
			}

			this->game->registerStage("game", new GameEngine(this->game, mapWidth, mapHeight, playerTeam));
			this->game->pushRegisteredStage("game");
		}
		break;
		case NextStageStr("continue_game"):
			this->game->pushRegisteredStage("game");
			break;
		}

	}
};