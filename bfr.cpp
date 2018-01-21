#include "GameEngine.hpp"
#include "MainMenu.hpp"
#include "PlayMenu.hpp"
#include "GameOver.hpp"
#include "Game.hpp"

int main()
{
	Game game(800,600,false);

	game.registerStage("main_menu", new MainMenu(&game));
	game.registerStage("play_menu", new PlayMenu(&game));
	game.registerStage("game_over", new GameOver(&game));

/*
	Player p;
	p.resources = 1034;
	p.butchery = 452;
	GameOver *go=(GameOver *)game.getStage("game_over");
	go->player = p;
    game.pushRegisteredStage("game_over");
*/
    game.pushRegisteredStage("main_menu");
    game.loop();

//	sf::sleep(sf::milliseconds(100));
    return 0;
}