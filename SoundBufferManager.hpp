#pragma once

#include <SFML/Audio.hpp>
#include <string>
#include <map>

class SoundBufferManager
{
private:

    /* Array of textures used */
    std::map<std::string, sf::SoundBuffer> buffers;

public:


    void loadSoundBuffer(std::string name, const std::string& filename)
    {
        /* Load the texture */
        sf::SoundBuffer buf;
        buf.loadFromFile(filename);

        /* Add it to the list of textures */
        this->buffers[name] = buf;

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
