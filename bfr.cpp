#include "GameEngine.hpp"
#include "MainMenu.hpp"
#include "Game.hpp"

int main()
{
	Game game(800,600);

    game.pushStage(new MainMenu(&game));
    game.loop();

    return 0;
}