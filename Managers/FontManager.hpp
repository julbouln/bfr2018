#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <map>

#include "Config.hpp"

class FontManager
{
private:
    std::map<std::string, sf::Font> fonts;

public:
    void load(std::string name, const std::string& filename)
    {
        if (this->fonts.count(name) == 0) {
            sf::Font font;
#ifdef MANAGER_DEBUG
            std::cout << "FontManager: load " << name << " from file " << filename << std::endl;
#endif
            font.loadFromFile(filename);
            this->fonts[name] = font;
        }
#ifdef MANAGER_DEBUG
        else {
            std::cout << "FontManager: " << name << " already loaded" << std::endl;
        }
#endif

        return;
    }

    sf::Font& getRef(std::string name)
    {
        return this->fonts.at(name);
    }

    bool hasRef(std::string name) {
        return this->fonts.count(name) > 0;
    }

};
