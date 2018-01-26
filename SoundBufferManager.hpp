#pragma once

#include <SFML/Audio.hpp>
#include <string>
#include <map>

#include "Config.hpp"

class SoundBufferManager
{
private:

    /* Array of textures used */
    std::map<std::string, sf::SoundBuffer> buffers;

public:


    void loadSoundBuffer(std::string name, const std::string& filename)
    {
#ifdef MANAGER_DEBUG
        std::cout << "SoundBufferManager: load " << name << " from file " << filename << std::endl;
#endif
        /* Load the texture */
        if (this->buffers.count(name) == 0) {
            sf::SoundBuffer buf;
            buf.loadFromFile(filename);

            /* Add it to the list of textures */
            this->buffers[name] = buf;
        }

        return;
    }

    sf::SoundBuffer& getRef(std::string texture)
    {
        return this->buffers.at(texture);
    }

    /* Constructor */
    SoundBufferManager()
    {
    }
};
