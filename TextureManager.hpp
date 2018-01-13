#ifndef TEXTURE_MANAGER_HPP
#define TEXTURE_MANAGER_HPP

#include "entt/entt.hpp"

#include <SFML/Graphics.hpp>
#include <string>
#include <map>

#include "Entity.hpp"

class TextureManager
{
private:

    /* Array of textures used */
    std::map<std::string, sf::Texture> textures;

public:

    void loadTexture(std::string name, int w, int h);

    /* Add a texture from a file */
    void loadTexture(std::string name, const std::string &filename);

    /* Add a texture from a image */
    void loadTexture(std::string name, sf::Image img, const sf::IntRect &area);

    /* Translate an id into a reference */
    sf::Texture& getRef(std::string texture);

    /* Constructor */
    TextureManager()
    {
    }
};

#endif /* TEXTURE_MANAGER_HPP */
