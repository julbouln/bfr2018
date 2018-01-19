#pragma once

#include "Game.hpp"
#include "GameStage.hpp"
#include "GameEngine.hpp"

class PlayMenu : public GameStage {
public:
	int team;
	int mapSize;

	sf::Sprite background;
	void draw(float dt) {
		background.setPosition(sf::Vector2f(0, 0));
		background.setScale(this->width / 800.0, this->height / 600.0);
		this->game->window.draw(background);

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.3f));
		ImGui::SetNextWindowPosCenter();
		if (ImGui::Begin("Play menu", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
		{
			ImVec2 sz(192, 64);

			ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor(0, 0, 0, 255) );

			ImGui::Text("Team"); ImGui::SameLine();
			ImGui::RadioButton("Rebel", &team, 0); ImGui::SameLine();
			ImGui::RadioButton("Neonaz", &team, 1);

			const char* mapSizes[] = { "Small", "Medium", "Large" };
			ImGui::Text("Map size"); ImGui::SameLine();
			ImGui::Combo("", &mapSize, mapSizes, IM_ARRAYSIZE(mapSizes));   // Combo using proper array. You can also pass a callback to retrieve array value, no need to create/copy an array just for that.

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(16, 16));
			if (ImGui::Button("Play", sz)) {
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
					mapWidth = 32;
					mapHeight = 32;
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

				this->game->pushStage(new GameEngine(this->game, mapWidth, mapHeight, playerTeam));
			}
			ImGui::PopStyleVar();

			ImGui::PopStyleColor();

			ImGui::End();
		}
		ImGui::PopStyleColor();


		ImGui::SFML::Render(this->game->window);
	}

	void update(float dt) {
	}

	void handleEvent(sf::Event &event) {
	}

	PlayMenu(Game *game) {
		this->game = game;

		this->setSize(this->game->width, this->game->height);

		this->initEffects();
		this->fadeOut();

		background.setTexture(this->game->vault.factory.getTex("intro_background"));

		this->team = 0;
		this->mapSize = 0;
	}
};