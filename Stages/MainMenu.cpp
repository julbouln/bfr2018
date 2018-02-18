#include "MainMenu.hpp"

MainMenu::MainMenu(Game *game) {
	this->game = game;

	this->setSize(this->game->width, this->game->height);

	this->initEffects();
	this->fadeIn();

	background.setTexture(this->game->vault.factory.getTex("intro_background"));
}

void MainMenu::draw(float dt) {
	background.setPosition(sf::Vector2f(0, 0));
	background.setScale(this->width / 800.0, this->height / 600.0);
	this->game->window.draw(background);

	this->guiPushStyles();

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	ImGui::SetNextWindowPosCenter();
	if (ImGui::Begin("Main menu", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
	{
		ImVec2 sz(192, 64);

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(16, 16));
		if (ImGui::Button("Play", sz)) {
			nextStage = NextStageStr("play_menu");
			this->fadeOut();
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

		ImGui::End();
	}
	ImGui::PopStyleColor();

	this->guiPopStyles();

	ImGui::SFML::Render(this->game->window);
	this->drawVersion();
	this->updateFading();
}

void MainMenu::update(float dt) {
}

void MainMenu::handleEvent(sf::Event &event) {
}

void MainMenu::reset() {
	this->fadeIn();
	this->nextStage = 0;
}

void MainMenu::fadeOutCallback() {
	switch (nextStage) {
	case NextStageStr("play_menu"):
		this->game->pushRegisteredStage("play_menu");
		break;
	}
}