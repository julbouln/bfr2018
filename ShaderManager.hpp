#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <map>

#include "Config.hpp"

class ShaderManager
{
private:
    std::map<std::string, sf::Shader*> shaders;

public:
    void load(std::string name, const std::string& filename)
    {
        if (this->shaders.count(name) == 0) {
            sf::Shader *shader = new sf::Shader();
#ifdef MANAGER_DEBUG
            std::cout << "ShaderManager: load " << name << " from file " << filename << std::endl;
#endif
            if (!shader->loadFromFile(filename, sf::Shader::Fragment)) {
                std::cout << "ERROR: cannot load " << filename << " shader" << std::endl;
            }
            this->shaders[name] = shader;
        }
#ifdef MANAGER_DEBUG
        else {
            std::cout << "ShaderManager: " << name << " already loaded" << std::endl;
        }
#endif

        return;
    }

    sf::Shader* getRef(std::string name)
    {
        return this->shaders.at(name);
    }

    bool hasRef(std::string name) {
        return this->shaders.count(name) > 0;
    }

    ~ShaderManager() {
        for(auto pair : this->shaders) {
            delete pair.second;
        }
    }

};
