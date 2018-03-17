#pragma once

#include <iostream>
#include <vector>
#include <math.h>
#include <string>
#include <map>
#include <random>
#include <stack>

#include <SFML/Graphics.hpp>

#include "third_party/imgui/imgui.h"
#include "third_party/imgui/imgui-sfml.h"
#include "third_party/imgui/imgui-sfml-extra.h"

#include "Stages/Stage.hpp"
#include "GameVault.hpp"

enum Cursor {
	DefaultCursor,
	RebelCursor,
	RebelCursor2,
	NeonazCursor,
	ScrollSouthCursor,
	ScrollNorthCursor,
	ScrollWestCursor,
	ScrollEastCursor,
	AttackCursor
};


class Game {
public:
	unsigned int width, height;

	std::stack<Stage*> stages;
	std::map<std::string, Stage*>registeredStages;

	GameVault vault;

	sf::RenderWindow window;

	bool mousePressed;
	bool mouseDoubleClick;
	int currentCursor;
	sf::Sprite cursor;

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
		this->currentCursor = DefaultCursor;
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
		sf::Clock mouseClickClock;
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

				if (event.type == sf::Event::MouseButtonPressed) {
					mousePressed = true;
					if(mouseClickClock.getElapsedTime().asSeconds() < 0.5f) {
						this->mouseDoubleClick = true;
						std::cout << "Mouse double click"<<std::endl;
					} else {
						this->mouseDoubleClick = false;
						mouseClickClock.restart();
					}
				}

				if (event.type == sf::Event::MouseButtonReleased) {
					mousePressed = false;
				}

				peekStage()->handleEvent(event);
			}

			peekStage()->update(elapsed);

			// clear the window with black color
			window.clear(sf::Color::Black);

			peekStage()->draw(elapsed);

			sf::IntRect cursorRect(this->currentCursor * 30, mousePressed ? 30 : 0, 30, 30);

			cursor.setTextureRect(cursorRect);
			cursor.setPosition(static_cast<sf::Vector2f>(sf::Mouse::getPosition(window)));
			window.draw(cursor);

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

		this->window.setMouseCursorVisible(false); // Hide cursor

		this->window.setFramerateLimit(30);
//		this->window.setVerticalSyncEnabled(true);

		srand (time(NULL));

		ImGui::SFML::Init(window, false);

		ImGuiIO& io = ImGui::GetIO();

		io.Fonts->Clear(); // clear fonts if you loaded some before (even if only default one was loaded)
		io.Fonts->AddFontFromFileTTF("medias/fonts/samos.ttf", 16.f);
		io.Fonts->AddFontFromFileTTF("medias/fonts/samos.ttf", 32.f);
		io.Fonts->AddFontFromFileTTF("medias/fonts/Vera.ttf", 14.f);
		io.Fonts->AddFontDefault(); // this will load default font as well
		ImGui::SFML::UpdateFontTexture();

		window.clear(sf::Color::Black);
		window.display();

		vault.factory.loadInitial();

		this->mousePressed = false;
		this->mouseDoubleClick = false;
		this->currentCursor = DefaultCursor;
//		cursor.setOrigin(15, 15);
		cursor.setTexture(vault.factory.getTex("cursors"));
		sf::IntRect cursorRect(this->currentCursor * 30, 0, 30, 30);
		cursor.setTextureRect(cursorRect);

	}

	~Game()
	{
		while (!this->stages.empty()) popStage();
	}
};
