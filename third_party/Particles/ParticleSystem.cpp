#include <iostream>
#include "ParticleSystem.h"

#include "ParticleData.h"
#include "ParticleHelpers.h"

namespace particles {

//#define USE_EMBEDDED_RENDER_TEXTURE

/* ParticleSystem */

ParticleSystem::ParticleSystem(int maxCount) : emitRate(0.f), m_dt(0.f) {
	m_particles = new ParticleData(maxCount);
}

ParticleSystem::~ParticleSystem() {
	delete m_particles;

	for (auto s : m_spawners) {
		delete s;
	}

	for (auto g : m_generators) {
		delete g;
	}

	for (auto u : m_updaters) {
		delete u;
	}
}

void ParticleSystem::removeGenerator(ParticleGenerator *g) {
	if (g == nullptr) return;
	auto it = std::find(m_generators.begin(), m_generators.end(), g);
	if (it == m_generators.end()) return;
	m_generators.erase(it);
	delete g;
}

void ParticleSystem::removeSpawner(ParticleSpawner *s) {
	if (s == nullptr) return;
	auto it = std::find(m_spawners.begin(), m_spawners.end(), s);
	if (it == m_spawners.end()) return;
	m_spawners.erase(it);
	delete s;
}

void ParticleSystem::removeUpdater(ParticleUpdater *u) {
	if (u == nullptr) return;
	auto it = std::find(m_updaters.begin(), m_updaters.end(), u);
	if (it == m_updaters.end()) return;
	m_updaters.erase(it);
	delete u;
}

void ParticleSystem::emitWithRate(float dt) {
	m_dt += dt;

	int maxNewParticles = 0;

	if (m_dt * emitRate > 1.0f) {
		maxNewParticles = static_cast<int>(m_dt * emitRate);
		m_dt -= maxNewParticles / emitRate;
	}

	if (maxNewParticles == 0) return;

	emitParticles(maxNewParticles);
}

void ParticleSystem::emitParticles(int count) {
	if (m_spawners.size() == 0) return;

	const int startId = m_particles->countAlive;
	const int endId = std::min(startId + count, m_particles->count - 1);
	const int newParticles = endId - startId;

	const int nSpawners = static_cast<int>(m_spawners.size());
	const int spawnerCount = newParticles / nSpawners;
	const int remainder = newParticles - spawnerCount * nSpawners;
	int spawnerStartId = startId;
	for (int i = 0; i < nSpawners; ++i) {
		int numberToSpawn = (i < remainder) ? spawnerCount + 1 : spawnerCount;
		m_spawners[i]->spawn(m_particles, spawnerStartId, spawnerStartId + numberToSpawn);
		spawnerStartId += numberToSpawn;
	}

	for (auto &generator : m_generators) {
		generator->generate(m_particles, startId, endId);
	}

	m_particles->countAlive += newParticles;
}

void ParticleSystem::update(float dt) {
	if (emitRate > 0.0f) {
		emitWithRate(dt);
	}

	for (int i = 0; i < m_particles->countAlive; ++i) {
		m_particles->acc[i] = { 0.0f, 0.0f };
	}

	for (auto & updater : m_updaters) {
		updater->update(m_particles, dt);
	}
}

void ParticleSystem::reset() {
	m_particles->countAlive = 0;
}

int ParticleSystem::countAlive() {
	return m_particles->countAlive;
}


/* PointParticleSystem */

PointParticleSystem::PointParticleSystem(int maxCount) : ParticleSystem(maxCount) {
	m_vertices = sf::VertexArray(sf::Points, maxCount);
}

void PointParticleSystem::render(sf::RenderTarget &renderTarget) {
	updateVertices();

	if (m_particles->countAlive <= 0) return;

	sf::RenderStates states = sf::RenderStates::Default;

	const sf::Vertex *ver = &m_vertices[0];
	renderTarget.draw(ver, m_particles->countAlive, sf::Points, states);
}

void PointParticleSystem::render(sf::RenderTarget &renderTarget, sf::RenderTexture *p_renderTexture) {
	this->render(renderTarget);
}

void PointParticleSystem::updateVertices() {
	for (int i = 0; i < m_particles->countAlive; ++i) {
		m_vertices[i].position = m_particles->pos[i];
		m_vertices[i].color = m_particles->col[i];
	}
}


/* LineParticleSystem */

LineParticleSystem::LineParticleSystem(int maxCount) : ParticleSystem(maxCount) {
	m_vertices = sf::VertexArray(sf::Lines, maxCount * 2);
	point1 = sf::Vector2f(0.0, 0.0);
	point1 = sf::Vector2f(0.0, 1.0);
}

void LineParticleSystem::render(sf::RenderTarget &renderTarget) {
	updateVertices();

	if (m_particles->countAlive <= 0) return;

	sf::RenderStates states = sf::RenderStates::Default;

	const sf::Vertex *ver = &m_vertices[0];
	renderTarget.draw(ver, m_particles->countAlive, sf::Lines, states);
}

void LineParticleSystem::render(sf::RenderTarget &renderTarget, sf::RenderTexture *p_renderTexture) {
	this->render(renderTarget);
}

void LineParticleSystem::setPoints(sf::Vector2f p1, sf::Vector2f p2) {
	point1 = p1;
	point2 = p2;
}

void LineParticleSystem::updateVertices() {
	for (int i = 0; i < m_particles->countAlive; i++) {
		float size = 0.5f * m_particles->size[i].x;
		float angle = m_particles->angle[i].x;

		m_vertices[2 * i].position = m_particles->pos[i] + point1;
		m_vertices[2 * i].color = m_particles->col[i];

		m_vertices[2 * i + 1].position = point2 * size;
		m_vertices[2 * i + 1].color = m_particles->col[i];

		if (angle != 0.f) {
			float sin = std::sin(angle);
			float cos = std::cos(angle);

			float x = m_vertices[2 * i + 1].position.x;
			float y = m_vertices[2 * i + 1].position.y;

			m_vertices[2 * i + 1].position.x = cos * x - sin * y;
			m_vertices[2 * i + 1].position.y = sin * x + cos * y;
		}

		m_vertices[2 * i + 1].position += m_particles->pos[i];


	}
}

/* TextureParticleSystem */

TextureParticleSystem::TextureParticleSystem(int maxCount, sf::Texture *texture) : ParticleSystem(maxCount), m_texture(texture) {
	m_vertices = sf::VertexArray(sf::Quads, maxCount * 4);

	float x = static_cast<float>(m_texture->getSize().x);
	float y = static_cast<float>(m_texture->getSize().y);

	for (int i = 0; i < m_particles->count; ++i) {
		m_vertices[4 * i + 0].texCoords = sf::Vector2f(0.f, 0.f);
		m_vertices[4 * i + 1].texCoords = sf::Vector2f(x, 0.f);
		m_vertices[4 * i + 2].texCoords = sf::Vector2f(x, y);
		m_vertices[4 * i + 3].texCoords = sf::Vector2f(0.f, y);

		m_vertices[4 * i + 0].color = sf::Color::White;
		m_vertices[4 * i + 1].color = sf::Color::White;
		m_vertices[4 * i + 2].color = sf::Color::White;
		m_vertices[4 * i + 3].color = sf::Color::White;
	}

	additiveBlendMode = false;
	applyShader = false;
	shader = nullptr;
}

TextureParticleSystem::TextureParticleSystem(int maxCount, sf::Texture *texture, int windowWidth, int windowHeight) : TextureParticleSystem(maxCount, texture) {
#ifdef USE_EMBEDDED_RENDER_TEXTURE
	if (windowWidth > 0 && windowHeight > 0)
		m_renderTexture.create(windowWidth, windowHeight);
#endif
}

void TextureParticleSystem::setTexture(sf::Texture *texture) {
	m_texture = texture;

	float x = static_cast<float>(m_texture->getSize().x);
	float y = static_cast<float>(m_texture->getSize().y);

	for (int i = 0; i < m_particles->count; ++i) {
		m_vertices[4 * i + 0].texCoords = sf::Vector2f(0.f, 0.f);
		m_vertices[4 * i + 1].texCoords = sf::Vector2f(x, 0.f);
		m_vertices[4 * i + 2].texCoords = sf::Vector2f(x, y);
		m_vertices[4 * i + 3].texCoords = sf::Vector2f(0.f, y);
	}
}

void TextureParticleSystem::updateVertices() {
	for (int i = 0; i < m_particles->countAlive; ++i) {
		float size = 0.5f * m_particles->size[i].x;
		float angle = m_particles->angle[i].x;

		m_vertices[4 * i + 0].position.x = -size;	m_vertices[4 * i + 0].position.y = -size;
		m_vertices[4 * i + 1].position.x = +size;	m_vertices[4 * i + 1].position.y = -size;
		m_vertices[4 * i + 2].position.x = +size;	m_vertices[4 * i + 2].position.y = +size;
		m_vertices[4 * i + 3].position.x = -size;	m_vertices[4 * i + 3].position.y = +size;

		if (angle != 0.f) {
			float sin = std::sin(angle); float cos = std::cos(angle);

			for (int j = 0; j < 4; ++j) {
				float x = m_vertices[4 * i + j].position.x;
				float y = m_vertices[4 * i + j].position.y;

				m_vertices[4 * i + j].position.x = cos * x - sin * y;
				m_vertices[4 * i + j].position.y = sin * x + cos * y;
			}
		}

		m_vertices[4 * i + 0].position.x += m_particles->pos[i].x;	m_vertices[4 * i + 0].position.y += m_particles->pos[i].y;
		m_vertices[4 * i + 1].position.x += m_particles->pos[i].x;	m_vertices[4 * i + 1].position.y += m_particles->pos[i].y;
		m_vertices[4 * i + 2].position.x += m_particles->pos[i].x;	m_vertices[4 * i + 2].position.y += m_particles->pos[i].y;
		m_vertices[4 * i + 3].position.x += m_particles->pos[i].x;	m_vertices[4 * i + 3].position.y += m_particles->pos[i].y;

		m_vertices[4 * i + 0].color = m_particles->col[i];
		m_vertices[4 * i + 1].color = m_particles->col[i];
		m_vertices[4 * i + 2].color = m_particles->col[i];
		m_vertices[4 * i + 3].color = m_particles->col[i];
	}
}

void TextureParticleSystem::draw(sf::RenderTarget &renderTarget) {
	if (m_particles->countAlive <= 0) return;

	sf::RenderStates states = sf::RenderStates::Default;

	if (additiveBlendMode) {
		states.blendMode = sf::BlendAdd;
	}

	states.texture = m_texture;

	const sf::Vertex *ver = &m_vertices[0];
	renderTarget.draw(ver, m_particles->countAlive * 4, sf::Quads, states);
}

void TextureParticleSystem::preRender(sf::RenderTexture *p_renderTexture) {
	sf::RenderStates states = sf::RenderStates::Default;

	states.blendMode = sf::BlendAdd;
	states.texture = m_texture;

	const sf::Vertex *ver = &m_vertices[0];

	p_renderTexture->clear(sf::Color(0, 0, 0, 0));
	p_renderTexture->draw(ver, m_particles->countAlive * 4, sf::Quads, states);
	p_renderTexture->display();
}

void TextureParticleSystem::drawWithShader(sf::RenderTarget &renderTarget, sf::RenderTexture *p_renderTexture) {
	sf::View oldView = renderTarget.getView();
	sf::View defaultView = renderTarget.getDefaultView();

	p_renderTexture->setView(oldView);
	this->preRender(p_renderTexture);
	m_sprite.setTexture(p_renderTexture->getTexture());

	applyShaderOptions(shader, shaderOptions);

	renderTarget.setView(defaultView);
	renderTarget.draw(m_sprite, shader);
	renderTarget.setView(oldView);
}

void TextureParticleSystem::drawWithShader(sf::RenderTarget &renderTarget) {
	if (m_particles->countAlive <= 0) return;

#ifdef USE_EMBEDDED_RENDER_TEXTURE
	sf::View oldView = renderTarget.getView();
	sf::View defaultView = renderTarget.getDefaultView();

	sf::RenderTexture *p_renderTexture = &m_renderTexture;

	p_renderTexture->setView(oldView);
	this->preRender(p_renderTexture);
	m_sprite.setTexture(p_renderTexture->getTexture());

	applyShaderOptions(shader, shaderOptions);

	renderTarget.setView(defaultView);
	renderTarget.draw(m_sprite, shader);
	renderTarget.setView(oldView);
#else
	sf::RenderStates states = sf::RenderStates::Default;
	states.blendMode = sf::BlendAdd;
	states.texture = m_texture;

	const sf::Vertex *ver = &m_vertices[0];

	applyShaderOptions(shader, shaderOptions);
	states.shader = shader;
	renderTarget.draw(ver, m_particles->countAlive * 4, sf::Quads, states);
#endif
}

void TextureParticleSystem::render(sf::RenderTarget &renderTarget) {
	updateVertices();
	if (applyShader)
		this->drawWithShader(renderTarget);
	else
		this->draw(renderTarget);
}

void TextureParticleSystem::render(sf::RenderTarget &renderTarget, sf::RenderTexture *p_renderTexture) {
	updateVertices();
	if (applyShader)
		this->drawWithShader(renderTarget, p_renderTexture);
	else
		this->draw(renderTarget);
}

/* SpriteSheetParticleSystem */

void SpriteSheetParticleSystem::render(sf::RenderTarget &renderTarget) {
	updateVertices();
	if (applyShader)
		this->drawWithShader(renderTarget);
	else
		this->draw(renderTarget);
}

void SpriteSheetParticleSystem::render(sf::RenderTarget &renderTarget, sf::RenderTexture *p_renderTexture) {
	updateVertices();
	if (applyShader)
		this->drawWithShader(renderTarget, p_renderTexture);
	else
		this->draw(renderTarget);
}

void SpriteSheetParticleSystem::updateVertices() {
	TextureParticleSystem::updateVertices();

	for (int i = 0; i < m_particles->countAlive; ++i) {
		float left = static_cast<float>(m_particles->texCoords[i].left);
		float top = static_cast<float>(m_particles->texCoords[i].top);
		float width = static_cast<float>(m_particles->texCoords[i].width);
		float height = static_cast<float>(m_particles->texCoords[i].height);

		m_vertices[4 * i + 0].texCoords = sf::Vector2f(left, top);
		m_vertices[4 * i + 1].texCoords = sf::Vector2f(left + width, top);
		m_vertices[4 * i + 2].texCoords = sf::Vector2f(left + width, top + height);
		m_vertices[4 * i + 3].texCoords = sf::Vector2f(left, top + height);
	}
}


/* MetaballParticleSystem */

MetaballParticleSystem::MetaballParticleSystem(int maxCount, sf::Texture *texture, sf::Shader *pShader) : TextureParticleSystem(maxCount, texture) {
	additiveBlendMode = true;
#if SFML_VERSION_MAJOR==2 && SFML_VERSION_MINOR > 3
	pShader->setUniform("texture", sf::Shader::CurrentTexture);
#else
	pShader->setParameter("texture", sf::Shader::CurrentTexture);
#endif
	applyShader = true;
	shader = pShader;
}

MetaballParticleSystem::MetaballParticleSystem(int maxCount, sf::Texture *texture, int windowWidth, int windowHeight) : TextureParticleSystem(maxCount, texture) {
	additiveBlendMode = true;
#if SFML_VERSION_MAJOR==2 && SFML_VERSION_MINOR > 3
	m_shader.setUniform("texture", sf::Shader::CurrentTexture);
#else
	m_shader.setParameter("texture", sf::Shader::CurrentTexture);
#endif

	if (!m_shader.loadFromMemory(metaballVertexShader, metaballFragmentShader)) {
		std::cerr << "MetaballParticleSystem: cannot load shader" << std::endl;
	}
#ifdef USE_EMBEDDED_RENDER_TEXTURE
	m_renderTexture.create(windowWidth, windowHeight);
#endif

	applyShader = true;
	shader = &m_shader;
}


MetaballParticleSystem::MetaballParticleSystem(int maxCount, sf::Texture *texture, int windowWidth, int windowHeight, sf::Shader *pShader) : TextureParticleSystem(maxCount, texture) {
	additiveBlendMode = true;
#if SFML_VERSION_MAJOR==2 && SFML_VERSION_MINOR > 3
	pShader->setUniform("texture", sf::Shader::CurrentTexture);
#else
	pShader->setParameter("texture", sf::Shader::CurrentTexture);
#endif
#ifdef USE_EMBEDDED_RENDER_TEXTURE
	m_renderTexture.create(windowWidth, windowHeight);
#endif
	applyShader = true;
	shader = pShader;
}

void MetaballParticleSystem::render(sf::RenderTarget &renderTarget) {
	updateVertices();
	// always render with shader
	shaderOptions.colors["customColor"] = color;
	shaderOptions.floats["threshold"] = threshold;
	this->drawWithShader(renderTarget);
}

void MetaballParticleSystem::render(sf::RenderTarget &renderTarget, sf::RenderTexture *p_renderTexture) {
	updateVertices();
	// always render with shader
	shaderOptions.colors["customColor"] = color;
	shaderOptions.floats["threshold"] = threshold;
	this->drawWithShader(renderTarget, p_renderTexture);
}

}