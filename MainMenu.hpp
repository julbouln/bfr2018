#pragma once

#include "Game.hpp"
#include "GameStage.hpp"
#include "PlayMenu.hpp"

class MainMenu : public GameStage {
public:
	sf::Sprite background;
	void draw(float dt) {
		background.setPosition(sf::Vector2f(0, 0));
		background.setScale(this->width / 800.0, this->height / 600.0);
		this->game->window.draw(background);

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.3f));
		ImGui::SetNextWindowPosCenter();
		if (ImGui::Begin("Main menu", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
		{
			ImVec2 sz(192, 64);

			ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor(0, 0, 0, 255) );
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(16, 16));
			if (ImGui::Button("Play", sz)) {
				this->game->pushStage(new PlayMenu(this->game));
			}

			if (ImGui::Button("Settings", sz)) {
			}

			if (ImGui::Button("Intro", sz)) {
			}

			if (ImGui::Button("Credits", sz)) {
			}

			if (ImGui::Button("Quit", sz)) {
				this->game->window.close();
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

	MainMenu(Game *game) {
		this->game = game;

		this->setSize(this->game->width, this->game->height);

		this->initEffects();
		this->fadeOut();

		background.setTexture(this->game->vault.factory.getTex("intro_background"));

	}
};