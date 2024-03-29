#pragma once

#include "Stage.hpp"
#include "Game.hpp"

typedef entt::HashedString::hash_type NextStage;
typedef entt::HashedString NextStageStr;

class GameStage : public Stage {
public:

	unsigned int width;
	unsigned int height;

	sf::RectangleShape fade;
	int fadeStep;
	int fadeType;
	int fadeSpeed;

	NextStage nextStage;

	sf::Text version;
	sf::Text text;

	GameStage() {
		version.setCharacterSize(24);
#if SFML_VERSION_MAJOR==2 && SFML_VERSION_MINOR > 3
		version.setFillColor(sf::Color::White);
#else
// SFML 2.3
		version.setColor(sf::Color::White);
#endif

		version.setString("BFR " + std::string(VERSION));
	}

	void drawVersion() {
		version.setFont(this->game->vault.factory.fntManager.getRef("samos"));
		version.setPosition(sf::Vector2f(8, this->height - 24 - 8));
		this->game->window.draw(version);
	}

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

		if (fadeType == 1 && fadeStep >= 255) {
			fadeOutCallback();
		}
	}

	void initEffects() {
		fade.setPosition(sf::Vector2f(0, 0));
		fade.setFillColor(sf::Color(0, 0, 0, 255));
		fade.setSize(sf::Vector2f(this->width, this->height));
		fadeSpeed = 8;
	}

	void guiPushStyles() {
//		this->setStyle();

		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

		ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor(0, 0, 0, 255) );
		ImGui::PushStyleColor(ImGuiCol_WindowBg, (ImVec4)ImColor(141, 98, 42, 255));

		ImGui::PushStyleColor(ImGuiCol_Border, (ImVec4)ImColor(96, 76, 29, 255));
		ImGui::PushStyleColor(ImGuiCol_BorderShadow, (ImVec4)ImColor(96, 76, 29, 255));
		ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4)ImColor(153, 106, 56, 255));
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, (ImVec4)ImColor(171, 119, 62, 255));
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive, (ImVec4)ImColor(141, 98, 42, 255));

		ImGui::PushStyleColor(ImGuiCol_PopupBg, (ImVec4)ImColor(153, 106, 56, 255));

		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(153, 106, 56, 255));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor(171, 119, 62, 255));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor(141, 98, 42, 255));

		ImGui::PushStyleColor(ImGuiCol_Header, (ImVec4)ImColor(96, 76, 29, 255));
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, (ImVec4)ImColor(171, 119, 62, 255));
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, (ImVec4)ImColor(141, 98, 42, 255));
	}

	void guiPopStyles() {
		ImGui::PopStyleColor(14);
		ImGui::PopStyleVar(2);
	}

	virtual void fadeOutCallback() = 0;

};