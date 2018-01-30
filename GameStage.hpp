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

		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

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
		ImGui::PopStyleColor(13);
		ImGui::PopStyleVar(2);
	}

	void setStyle() {
		ImGuiStyle& style = ImGui::GetStyle();

		style.FrameBorderSize = 2.f;
		style.FrameRounding = 4.f;

		style.Colors[ImGuiCol_Text]                  = (ImVec4)ImColor(255, 255, 255, 255);
		style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		style.Colors[ImGuiCol_WindowBg]              = (ImVec4)ImColor(153, 106, 56, 255);
		style.Colors[ImGuiCol_ChildWindowBg]         = (ImVec4)ImColor(153, 106, 56, 255);
		style.Colors[ImGuiCol_PopupBg]               = (ImVec4)ImColor(153, 106, 56, 255);
		style.Colors[ImGuiCol_Border]                = (ImVec4)ImColor(96, 76, 29, 255);
		style.Colors[ImGuiCol_BorderShadow]          = (ImVec4)ImColor(96, 76, 29, 255);
		style.Colors[ImGuiCol_FrameBg]               = (ImVec4)ImColor(153, 106, 56, 255);
		style.Colors[ImGuiCol_FrameBgHovered]        = (ImVec4)ImColor(171, 119, 62, 255);
		style.Colors[ImGuiCol_FrameBgActive]         = (ImVec4)ImColor(141, 98, 42, 255);
		style.Colors[ImGuiCol_TitleBg]               = (ImVec4)ImColor(153, 106, 56, 255);
		style.Colors[ImGuiCol_TitleBgCollapsed]      = (ImVec4)ImColor(153, 106, 56, 255);
		style.Colors[ImGuiCol_TitleBgActive]         = (ImVec4)ImColor(153, 106, 56, 255);
		style.Colors[ImGuiCol_MenuBarBg]             = (ImVec4)ImColor(153, 106, 56, 255);
		style.Colors[ImGuiCol_ScrollbarBg]           = (ImVec4)ImColor(153, 106, 56, 255);
		style.Colors[ImGuiCol_ScrollbarGrab]         = (ImVec4)ImColor(153, 106, 56, 255);
		style.Colors[ImGuiCol_ScrollbarGrabHovered]  = (ImVec4)ImColor(153, 106, 56, 255);
		style.Colors[ImGuiCol_ScrollbarGrabActive]   = (ImVec4)ImColor(153, 106, 56, 255);
		//style.Colors[ImGuiCol_ComboBg]               = ImVec4(0.32f, 0.32f, 0.32f, 1.00f);
		style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.78f, 0.78f, 0.78f, 1.00f);
		style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.74f, 0.74f, 0.74f, 1.00f);
		style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.74f, 0.74f, 0.74f, 1.00f);
		style.Colors[ImGuiCol_Button]                = (ImVec4)ImColor(153, 106, 56, 255);
		style.Colors[ImGuiCol_ButtonHovered]         = (ImVec4)ImColor(171, 119, 62, 255);
		style.Colors[ImGuiCol_ButtonActive]          = (ImVec4)ImColor(141, 98, 42, 255);
		style.Colors[ImGuiCol_Header]                = (ImVec4)ImColor(141, 98, 42, 255);
		style.Colors[ImGuiCol_HeaderHovered]         = (ImVec4)ImColor(171, 119, 62, 255);
		style.Colors[ImGuiCol_HeaderActive]          = (ImVec4)ImColor(141, 98, 42, 255);
		style.Colors[ImGuiCol_Column]                = (ImVec4)ImColor(153, 106, 56, 255);
		style.Colors[ImGuiCol_ColumnHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_ColumnActive]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(0.36f, 0.36f, 0.36f, 1.00f);
		style.Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_CloseButton]           = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
		style.Colors[ImGuiCol_CloseButtonHovered]    = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
		style.Colors[ImGuiCol_CloseButtonActive]     = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
		style.Colors[ImGuiCol_PlotLines]             = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
		style.Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.32f, 0.52f, 0.65f, 1.00f);
		style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.50f);
	}

	virtual void fadeOutCallback() = 0;

};