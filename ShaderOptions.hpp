#pragma once

struct ShaderOptions {
	std::map<std::string,sf::Color> colors;
	std::map<std::string,float> floats;
};

void applyShaderOptions(sf::Shader *shader, ShaderOptions &options);