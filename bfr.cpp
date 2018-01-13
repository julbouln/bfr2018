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

#include "GameEngine.hpp"

int main()
{
	int gameWidth = 800;
	int gameHeight = 600;
	entt::Registry<EntityID> registry;
	EntityID emptyEntity;
	EntityFactory factory;

	GameEngine engine;

	ImGuiIO& io = ImGui::GetIO();

	sf::RenderWindow window(sf::VideoMode(gameWidth, gameHeight), "BFR2018");
	engine.setSize(gameWidth, gameHeight);
	window.setFramerateLimit(60);

	ImGui::SFML::Init(window, false);

	io.Fonts->Clear(); // clear fonts if you loaded some before (even if only default one was loaded)
	// IO.Fonts->AddFontDefault(); // this will load default font as well
//	io.Fonts->AddFontFromFileTTF("medias/fonts/Vera.ttf", 14.f);
	io.Fonts->AddFontFromFileTTF("medias/fonts/samos.ttf", 16.f);
	ImGui::SFML::UpdateFontTexture();

	sf::Clock clock;

	emptyEntity = registry.create();

	engine.iface.setTexture(factory.ifaceRebelTex());

	engine.generate(registry, factory);
	engine.initView(window);

	while (window.isOpen())
	{
		sf::Event event;

		sf::Time elapsed = clock.restart();
		float dt = elapsed.asSeconds();
		engine.update(registry, factory, dt);
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
				engine.handleEvent(registry, window, event);
			}
		}

		window.setView(engine.gameView);

		// clear the window with black color
		window.clear(sf::Color::Black);
		engine.draw(registry, factory, window, dt);

		ImGui::SFML::Render(window);

		window.display();
	}
}