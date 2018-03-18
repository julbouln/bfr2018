#include "Settings.hpp"

Settings::Settings(Game *game) {
	this->game = game;

	this->setSize(this->game->width, this->game->height);

	this->initEffects();
	this->fadeIn();

	background.setTexture(this->game->vault.factory.getTex("intro_background"));

	this->screenSize = this->game->settings.screenSize;
	this->fullscreen = this->game->settings.fullscreen;
}

void Settings::draw(float dt) {
	background.setPosition(sf::Vector2f(0, 0));
	background.setScale(this->width / 800.0, this->height / 600.0);
	this->game->window.draw(background);

	this->guiPushStyles();

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	ImGui::SetNextWindowPosCenter();
	if (ImGui::Begin("Settings", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
	{
		ImVec2 sz(128, 32);

		const char* screenSizes[] = { "800x600", "1024x768", "1280x1024", "1600x900"};
		ImGui::Text("Screen size"); ImGui::SameLine();
		ImGui::Combo("", &screenSize, screenSizes, IM_ARRAYSIZE(screenSizes));   // Combo using proper array. You can also pass a callback to retrieve array value, no need to create/copy an array just for that.

		ImGui::Checkbox("Fullscreen", &fullscreen);

		if (ImGui::Button("Cancel", sz)) {
			nextStage = NextStageStr("main_menu");

			this->fadeOut();
		}; ImGui::SameLine();

		if (ImGui::Button("Save", sz)) {
			this->game->settings.screenSize = this->screenSize;
			this->game->settings.fullscreen = this->fullscreen;
			this->game->settings.save();
			nextStage = NextStageStr("main_menu");
			this->fadeOut();
		}

		ImGui::End();
	}
	ImGui::PopStyleColor();

	this->guiPopStyles();

	ImGui::SFML::Render(this->game->window);
	this->drawVersion();
	this->updateFading();
}

void Settings::update(float dt) {
}

void Settings::handleEvent(sf::Event &event) {
}

void Settings::reset() {
	this->fadeIn();
	this->nextStage = 0;
}

void Settings::fadeOutCallback() {
	switch (nextStage) {
	case NextStageStr("main_menu"):
		this->game->pushRegisteredStage("main_menu");
		break;
	}
}