#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <map>

#include "Config.hpp"

class TextureManager
{
private:

    /* Array of textures used */
    std::map<std::string, sf::Texture> textures;

public:

    void loadTexture(std::string name, int w, int h)
    {
        if (this->textures.count(name) == 0) {
            sf::Texture tex;
            tex.create(w, h);
            this->textures[name] = tex;
        }
        return;
    }

    void loadTexture(std::string name, const std::string& filename)
    {
#ifdef MANAGER_DEBUG
        std::cout << "TextureManager: load " << name << " from file " << filename << std::endl;
#endif

        if (this->textures.count(name) == 0) {
            /* Load the texture */
            sf::Texture tex;
            tex.loadFromFile(filename);

            /* Add it to the list of textures */
            this->textures[name] = tex;
        }
        return;
    }

    void loadTexture(std::string name, sf::Image img, const sf::IntRect &area) {
#ifdef MANAGER_DEBUG
        std::cout << "TextureManager: load " << name << " from image " << std::endl;
#endif

        if (this->textures.count(name) == 0) {
            /* Load the texture */
            sf::Texture tex;
            tex.loadFromImage(img, area);

            /* Add it to the list of textures */
            this->textures[name] = tex;
        }
        return;

    }

    sf::Texture& getRef(std::string texture)
    {
        return this->textures.at(texture);
    }

    /* Constructor */
    TextureManager()
    {
    }
};
