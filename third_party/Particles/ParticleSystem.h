#pragma once

#include <SFML/Graphics.hpp>

#include "ParticleGenerator.h"
#include "ParticleSpawner.h"
#include "ParticleUpdater.h"

#include "Options.hpp"

namespace particles {

class ParticleData;

/* Abstract base class for all particle system types */
class ParticleSystem : public sf::Transformable {
public:
	ParticleSystem(int maxCount);
	virtual ~ParticleSystem();

	ParticleSystem(const ParticleSystem &) = delete;
	ParticleSystem &operator=(const ParticleSystem &) = delete;

	void reset();

	virtual void update(float dt);
	virtual void render(sf::RenderTarget &renderTarget) = 0;
	virtual void render(sf::RenderTarget &renderTarget, sf::RenderTexture *p_renderTexture) = 0;

	template<typename T>
	inline T *addGenerator() {
		T *g = new T();
		m_generators.push_back(g);
		return g;
	}

	template<typename T>
	inline T *addSpawner() {
		T *s = new T();
		m_spawners.push_back(s);
		return s;
	}

	template<typename T>
	inline T *addUpdater() {
		T *u = new T();
		m_updaters.push_back(u);
		return u;
	}

	void removeGenerator(ParticleGenerator *g);
	void removeSpawner(ParticleSpawner *s);
	void removeUpdater(ParticleUpdater *u);

	void emitParticles(int count); 	// emit a fix number of particles

	inline size_t getNumberGenerators() const { return m_generators.size(); }
	inline size_t getNumberSpawners() const { return m_spawners.size(); }
	inline size_t getNumberUpdaters() const { return m_updaters.size(); }

	inline void clearGenerators() { 
		for (auto g : m_generators) {
			delete g;
		}
		m_generators.clear();
	}

	inline void clearSpawners() {
		for (auto s : m_spawners) {
			delete s;
		}
		m_spawners.clear();
	}

	inline void clearUpdaters() {
		for (auto u : m_updaters) {
			delete u;
		}
		m_updaters.clear();
	}

	int countAlive();

protected:
	void emitWithRate(float dt);	// emit a stream of particles defined by emitRate and dt

public:
	float	emitRate;	// Note: For a constant particle stream, it should hold that: emitRate <= (maximalParticleCount / averageParticleLifetime)

protected:
	float m_dt;

	ParticleData *m_particles;
	
	std::vector<ParticleGenerator *> m_generators;
	std::vector<ParticleSpawner *> m_spawners;
	std::vector<ParticleUpdater *> m_updaters;

	sf::VertexArray m_vertices;
};


class PointParticleSystem : public ParticleSystem {
public:
	PointParticleSystem(int maxCount);
	virtual ~PointParticleSystem() {}

	PointParticleSystem(const PointParticleSystem &) = delete;
	PointParticleSystem &operator=(const PointParticleSystem &) = delete;

	virtual void render(sf::RenderTarget& renderTarget) override;
	virtual void render(sf::RenderTarget &renderTarget, sf::RenderTexture *p_renderTexture) override;

protected:
	void updateVertices();
};


class LineParticleSystem : public ParticleSystem {
public:
	LineParticleSystem(int maxCount);
	virtual ~LineParticleSystem() {}

	LineParticleSystem(const LineParticleSystem &) = delete;
	LineParticleSystem &operator=(const LineParticleSystem &) = delete;

	virtual void render(sf::RenderTarget& renderTarget) override;
	virtual void render(sf::RenderTarget &renderTarget, sf::RenderTexture *p_renderTexture) override;

	void setPoints(sf::Vector2f p1, sf::Vector2f p2);

protected:
	void updateVertices();
	sf::Vector2f point1;
	sf::Vector2f point2;
};



class TextureParticleSystem : public ParticleSystem {
public:
	TextureParticleSystem(int maxCount, sf::Texture *texture);
	TextureParticleSystem(int maxCount, sf::Texture *texture, int windowWidth, int windowHeight);
	virtual ~TextureParticleSystem() {}

	TextureParticleSystem(const TextureParticleSystem &) = delete;
	TextureParticleSystem &operator=(const TextureParticleSystem &) = delete;

	virtual void render(sf::RenderTarget &renderTarget) override;
	virtual void render(sf::RenderTarget &renderTarget, sf::RenderTexture *p_renderTexture) override;

	// extra
	void draw(sf::RenderTarget &renderTarget);
	void drawWithShader(sf::RenderTarget &renderTarget);

	void setTexture(sf::Texture *texture);

	void preRender(sf::RenderTexture *p_renderTexture);
	void drawWithShader(sf::RenderTarget &renderTarget, sf::RenderTexture *p_renderTexture);

protected:
	void updateVertices();

public:
	bool additiveBlendMode;
// extra shader
	bool applyShader;
	sf::Shader *shader;
	ShaderOptions shaderOptions;

protected:
	sf::Sprite m_sprite;
	sf::RenderTexture m_renderTexture;
	sf::Texture *m_texture;
};


class SpriteSheetParticleSystem : public TextureParticleSystem {
public:
	SpriteSheetParticleSystem(int maxCount, sf::Texture *texture) : TextureParticleSystem(maxCount, texture) {}
	SpriteSheetParticleSystem(int maxCount, sf::Texture *texture, int windowWidth, int windowHeight) : TextureParticleSystem(maxCount, texture, windowWidth, windowHeight) {}
	virtual ~SpriteSheetParticleSystem() {}

	SpriteSheetParticleSystem(const SpriteSheetParticleSystem &) = delete;
	SpriteSheetParticleSystem &operator=(const SpriteSheetParticleSystem &) = delete;

	virtual void render(sf::RenderTarget &renderTarget) override;
	virtual void render(sf::RenderTarget &renderTarget, sf::RenderTexture *p_renderTexture) override;

protected:
	void updateVertices();
};


static const std::string metaballVertexShader = \
                                 "void main()" \
                                 "{" \
                                 "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;" \
                                 "    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;" \
                                 "    gl_FrontColor = gl_Color;" \
                                 "}";

static const std::string metaballFragmentShader = \
                                   "uniform sampler2D texture;" \
                                   "uniform vec4 customColor;" \
                                   "uniform float threshold;" \
                                   "" \
                                   "void main()" \
                                   "{" \
                                   "    vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);" \
                                   "    if (pixel.a > threshold) {" \
                                   "        gl_FragColor = customColor;" \
                                   "    }" \
                                   "    else {" \
                                   "        gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);" \
                                   "    }" \
                                   "}";


class MetaballParticleSystem : public TextureParticleSystem {
public:
	MetaballParticleSystem(int maxCount, sf::Texture *texture, sf::Shader *pShader);
	MetaballParticleSystem(int maxCount, sf::Texture *texture, int windowWidth, int windowHeight);
	MetaballParticleSystem(int maxCount, sf::Texture *texture, int windowWidth, int windowHeight, sf::Shader *pShader);
	virtual ~MetaballParticleSystem() {}

	MetaballParticleSystem(const MetaballParticleSystem &) = delete;
	MetaballParticleSystem &operator=(const MetaballParticleSystem &) = delete;

	virtual void render(sf::RenderTarget &renderTarget) override;
	virtual void render(sf::RenderTarget &renderTarget, sf::RenderTexture *p_renderTexture) override;

public:
	sf::Color color{ sf::Color::White };
	float threshold{ 0.5f };

protected:
	sf::Shader m_shader;
};

}