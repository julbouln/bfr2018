#include "GameEngine.hpp"
#include "MainMenu.hpp"
#include "PlayMenu.hpp"
#include "Game.hpp"

int main()
{
	Game game(800,600);

	game.registerStage("main_menu", new MainMenu(&game));
	game.registerStage("play_menu", new PlayMenu(&game));

    game.pushRegisteredStage("main_menu");
    game.loop();

//	sf::sleep(sf::milliseconds(100));
    return 0;
}