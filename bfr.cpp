#include "GameEngine.hpp"
#include "Game.hpp"
#include "Stages/MainMenu.hpp"
#include "Stages/PlayMenu.hpp"
#include "Stages/GameOver.hpp"

#include "third_party/argagg.hpp"

int main(int argc, char *argv[])
{
	bool fullscreen = false;
	unsigned int width = 1024;
	unsigned int height = 768;

	argagg::parser argparser {{
			{	"help", {"-h", "--help"},
				"shows this help message", 0
			},
			{	"fullscreen", {"-f", "--fullscreen"},
				"fullscreen mode", 0
			},
			{	"windowed", {"-w", "--windowed"},
				"windowed mode", 0
			},
			{	"width", {"-x", "--width"},
				"screen width", 1
			},
			{	"height", {"-y", "--height"},
				"screen height", 1
			},
			{	"version", {"-v", "--version"},
				"version", 0
			}
		}};

	argagg::parser_results args;
	try {
		args = argparser.parse(argc, argv);
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	if (args["help"]) {
		std::cerr << "Usage: bfr [options]" << std::endl
            << argparser;
		//     -h, --help
		//         shows this help message
		//     -d, --delim
		//         delimiter (default: ,)
		//     -n, --num
		//         number
		return EXIT_SUCCESS;
	}
	if (args["version"]) {
		std::cout << "Battle For Rashitoul "<<VERSION<<std::endl;
		return EXIT_SUCCESS;
	}

	if (args["fullscreen"]) {
		fullscreen = true;
	}

	if (args["windowed"]) {
		fullscreen = false;
	}

	if (args["width"]) {
		width = args["width"];
	}

	if (args["height"]) {
		height = args["height"];
	}

	Game game(width, height, fullscreen);

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

	return EXIT_SUCCESS;
}