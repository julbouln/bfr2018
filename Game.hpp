#pragma once

#include <iostream>
#include <vector>
#include <math.h>
#include <string>
#include <map>
#include <random>
#include <stack>

#include <SFML/Graphics.hpp>

#include "gui/imgui.h"
#include "gui/imgui-sfml.h"
#include "gui/imgui-sfml-extra.h"

#include "Stage.hpp"
#include "GameVault.hpp"

class Game {
public:
	std::stack<Stage*> stages;

	std::map<std::string, Stage*>registeredStages;

	unsigned int width, height;

	GameVault vault;

	sf::RenderWindow window;

	void registerStage(std::string name, Stage *stage) {
		this->registeredStages[name] = stage;
	}

	void unregisterStage(std::string name) {
//		delete this->registeredStages[name];
		this->registeredStages.erase(name);
	}

	Stage *getStage(std::string name) {
		return this->registeredStages[name];
	}

	void pushRegisteredStage(std::string name) {
		this->registeredStages[name]->reset();
		this->pushStage(this->registeredStages[name]);
	}

	bool isRegisteredStage(std::string name) {
		return this->registeredStages.count(name) > 0;
	}

	void pushStage(Stage* stage)
	{
		this->stages.push(stage);
		return;
	}

	void popStage()
	{
//		delete this->stages.top();
		this->stages.pop();
		return;
	}

	void changeStage(Stage* stage)
	{
		if (!this->stages.empty())
			popStage();
		pushStage(stage);
		return;
	}

	Stage* peekStage()
	{
		if (this->stages.empty()) return nullptr;
		return this->stages.top();
	}

	void loop()
	{
		sf::Clock clock;
		ImGuiIO& io = ImGui::GetIO();

		while (window.isOpen())
		{
			sf::Event event;

			sf::Time elapsed = clock.restart();
			float dt = elapsed.asSeconds();
			ImGui::SFML::Update(window, elapsed);

			while (window.pollEvent(event))
			{
				ImGui::SFML::ProcessEvent(event);

				// "close requested" event: we close the window
				if (event.type == sf::Event::Closed)
					window.close();

				if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
					window.close();

				peekStage()->handleEvent(event);
			}

			peekStage()->update(dt);

			// clear the window with black color
			window.clear(sf::Color::Black);

			peekStage()->draw(dt);

			window.display();
		}
	}


	Game(unsigned int width, unsigned int height, bool fullscreen)
	{
		this->width = width;
		this->height = height;
		if (fullscreen)
			this->window.create(sf::VideoMode(this->width, this->height), "BFR2018", sf::Style::Fullscreen);
		else
			this->window.create(sf::VideoMode(this->width, this->height), "BFR2018");

		this->window.setFramerateLimit(30);

		srand (time(NULL));

		ImGui::SFML::Init(window, false);

		ImGuiIO& io = ImGui::GetIO();

		io.Fonts->Clear(); // clear fonts if you loaded some before (even if only default one was loaded)
		io.Fonts->AddFontFromFileTTF("medias/fonts/samos.ttf", 16.f);
		io.Fonts->AddFontFromFileTTF("medias/fonts/Vera.ttf", 14.f);
		io.Fonts->AddFontDefault(); // this will load default font as well
		ImGui::SFML::UpdateFontTexture();

		window.clear(sf::Color::Black);
		window.display();

		vault.factory.load();
	}


	~Game()
	{
		while (!this->stages.empty()) popStage();
	}


};
