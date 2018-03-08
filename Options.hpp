#pragma once

#include "Managers/TextureManager.hpp"

struct ShaderOptions {
	std::map<std::string,sf::Color> colors;
	std::map<std::string,float> floats;
};

void applyShaderOptions(sf::Shader *shader, ShaderOptions &options);


class ParticleEffectOptions {
public:
	ParticleEffectOptions() {
		destPos = sf::Vector2f(-32.0, -32.0);
		texMgr = nullptr;
		direction = 0;
		applyShader = false;
		shader = nullptr;
	}

	bool hasDestPos() {
		if (destPos.x >= 0)
			return true;
		else
			return false;
	}

	TextureManager *texMgr;
	sf::Vector2f destPos; // for aimed swawner
	int direction; // for aimed spawner
	// shader
	bool applyShader;
	sf::Shader *shader;
	ShaderOptions shaderOptions;
};