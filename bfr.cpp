#include "GameEngine.hpp"

#include "Game.hpp"

/*
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
*/

int main()
{
//	int gameWidth = 800;
//	int gameHeight = 600;


//	GameEngine engine(&vault);




/*
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

*/



	Game game(800,600);

    game.pushStage(new GameEngine(&game));
    game.loop();

    return 0;
}