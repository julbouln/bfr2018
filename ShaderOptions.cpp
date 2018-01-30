#include <SFML/Graphics.hpp>
#include "ShaderOptions.hpp"

void applyShaderOptions(sf::Shader *shader, ShaderOptions &options) {
	#if SFML_VERSION_MAJOR==2 && SFML_VERSION_MINOR > 3
		shader->setUniform("texture", sf::Shader::CurrentTexture);
		for (auto pair : options.colors) {
			shader->setUniform(pair.first, sf::Glsl::Vec4(pair.second.r / 255.f, pair.second.g / 255.f, pair.second.b / 255.f, pair.second.a / 255.f));
		}
		for (auto pair : options.floats) {
			shader->setUniform(pair.first, pair.second);
		}
#else
// SFML 2.3
		shader->setParameter("texture", sf::Shader::CurrentTexture);
		for (auto pair : options.colors) {
			shader->setParameter(pair.first, pair.second);
		}
		for (auto pair : options.floats) {
			shader->setParameter(pair.first, pair.second);
		}
#endif

}