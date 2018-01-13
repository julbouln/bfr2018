#include <SFML/Graphics.hpp>
#include <map>
#include <string>

#include "TextureManager.hpp"

void TextureManager::loadTexture(std::string name, int w, int h)
{
    sf::Texture tex;
    tex.create(w, h);
    this->textures[name] = tex;
    return;
}

void TextureManager::loadTexture(std::string name, const std::string& filename)
{
    /* Load the texture */
    sf::Texture tex;
    tex.loadFromFile(filename);

    /* Add it to the list of textures */
    this->textures[name] = tex;

    return;
}

void TextureManager::loadTexture(std::string name, sf::Image img, const sf::IntRect &area) {
    /* Load the texture */
    sf::Texture tex;
    tex.loadFromImage(img, area);

    /* Add it to the list of textures */
    this->textures[name] = tex;

    return;

}

sf::Texture& TextureManager::getRef(std::string texture)
{    
    return this->textures.at(texture);
}
