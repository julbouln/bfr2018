#include <iostream>
#include <vector>
#include <math.h>
#include <string>
#include <map>
#include <random>

#include <SFML/Graphics.hpp>

#include "SimplexNoise.h"
#include "JPS.h"

#include "Components.hpp"
#include "EntityFactory.hpp"

#include "gui/imgui.h"
#include "gui/imgui-sfml.h"
#include "gui/imgui-sfml-extra.h"

#include "GameVault.hpp"
#include "GameEngine.hpp"


sf::Text genText(sf::Font &font, std::string str, int size) {
	sf::Text text;

// select the font
	text.setFont(font); // font is a sf::Font

// set the string to display
	text.setString(str);

// set the character size
	text.setCharacterSize(size); // in pixels, not points!

// set the color
	text.setColor(sf::Color::White);
	return text;
}


int main()
{
	int gameWidth = 800;
	int gameHeight = 600;

	GameVault vault;

//	entt::Registry<EntityID> registry;
//	EntityFactory factory;

	EntityID emptyEntity;
	GameEngine engine(&vault);

	ImGuiIO& io = ImGui::GetIO();

	sf::RenderWindow window(sf::VideoMode(gameWidth, gameHeight), "BFR2018");
	engine.setSize(gameWidth, gameHeight);
	window.setFramerateLimit(30);

	srand (time(NULL));

	ImGui::SFML::Init(window, false);

	sf::Font font;
	if (!font.loadFromFile("medias/fonts/samos.ttf"))
	{
		// error...
	}

	sf::RectangleShape fade;
	fade.setPosition(sf::Vector2f(0, 0));
	fade.setFillColor(sf::Color(0, 0, 0, 255));
	fade.setSize(sf::Vector2f(gameWidth, gameHeight));
	int fadeStep = 255;

	sf::Text loading = genText(font, "Loading", 48);
	sf::FloatRect textRect = loading.getLocalBounds();
	loading.setOrigin(textRect.left + textRect.width / 2.0f,
	                  textRect.top  + textRect.height / 2.0f);
	loading.setPosition(sf::Vector2f(gameWidth / 2, gameHeight / 2));

	window.draw(fade);
	window.draw(loading);
	window.display();

	io.Fonts->Clear(); // clear fonts if you loaded some before (even if only default one was loaded)
	io.Fonts->AddFontFromFileTTF("medias/fonts/samos.ttf", 16.f);
	io.Fonts->AddFontFromFileTTF("medias/fonts/Vera.ttf", 14.f);
	io.Fonts->AddFontDefault(); // this will load default font as well
	ImGui::SFML::UpdateFontTexture();


	vault.factory.load();

	sf::Clock clock;

	emptyEntity = vault.registry.create();

	engine.generate(64,64);
	engine.initView(window);

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

			if (event.key.code == sf::Keyboard::Escape)
				window.close();

			if (!io.WantCaptureMouse) { /* do not enable map interface if gui used */
				engine.handleEvent(window, event);
			}
		}

		engine.update(dt);

		window.setView(engine.gameView);

		// clear the window with black color
		window.clear(sf::Color::Black);
		engine.draw(window, dt);

		ImGui::SFML::Render(window);

		if (fadeStep > 0)
		{
			fadeStep -= 3;
			if (fadeStep < 0)
				fadeStep = 0;
			fade.setFillColor(sf::Color(0, 0, 0, fadeStep));
			window.draw(fade);
		}

		window.display();
	}
}