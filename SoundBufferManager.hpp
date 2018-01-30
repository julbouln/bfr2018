#pragma once

#include <SFML/Audio.hpp>
#include <string>
#include <map>

#include "Config.hpp"

class SoundBufferManager
{
private:
    std::map<std::string, sf::SoundBuffer> buffers;

public:
    void load(std::string name, const std::string& filename)
    {
        if (this->buffers.count(name) == 0) {
            sf::SoundBuffer buf;
#ifdef MANAGER_DEBUG
        std::cout << "SoundBufferManager: load " << name << " from file " << filename << std::endl;
#endif
            buf.loadFromFile(filename);

            this->buffers[name] = buf;
        } 
#ifdef MANAGER_DEBUG
        else {
            std::cout << "SoundBufferManager: " << name << " already loaded" << std::endl;
        }
#endif

        return;
    }

    sf::SoundBuffer& getRef(std::string name)
    {
        return this->buffers.at(name);
    }

    bool hasRef(std::string name) {
        return this->buffers.count(name) > 0;
    }

};
