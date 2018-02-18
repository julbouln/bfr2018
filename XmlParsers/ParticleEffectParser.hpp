#pragma once

#include "third_party/tinyxml2.h"
#include "third_party/tixml2ex.h"

#include "Managers/TextureManager.hpp"

#include "Components/ParticleEffect.hpp"

enum class ParticleSystemMode {
	Points,
	Lines,
	Texture,
	Spritesheet,
	AnimatedSpritesheet,
	Metaball
};

enum class SpawnerMode {
	Point,
	Box,
	Circle,
	Disk
};

enum class VelocityGeneratorMode {
	Angled,
	Vector,
	Aimed
};

static std::map<std::string, ParticleSystemMode> partSysModes =
{
	{ "points", ParticleSystemMode::Points },
	{ "lines", ParticleSystemMode::Lines },
	{ "texture", ParticleSystemMode::Texture },
	{ "spritesheet", ParticleSystemMode::Spritesheet },
	{ "animated_spritesheet", ParticleSystemMode::AnimatedSpritesheet },
	{ "metaball", ParticleSystemMode::Metaball },
};

static std::map<std::string, SpawnerMode> spawnModes =
{
	{ "point", SpawnerMode::Point },
	{ "box", SpawnerMode::Box },
	{ "circle", SpawnerMode::Circle },
	{ "disk", SpawnerMode::Disk },
};

static std::map<std::string, VelocityGeneratorMode> velGenModes =
{
	{ "angled", VelocityGeneratorMode::Angled },
	{ "vector", VelocityGeneratorMode::Vector },
	{ "aimed", VelocityGeneratorMode::Aimed },
};

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
	sf::Vector2i screenSize; // for metaball
	// shader
	bool applyShader;
	sf::Shader *shader;
	ShaderOptions shaderOptions;
};

class ParticleEffectParser {
	sf::Shader metaballShader;
public:

	ParticleEffectParser() {
		metaballShader.loadFromMemory(particles::metaballVertexShader, particles::metaballFragmentShader);
	}

	sf::Color parseColor(tinyxml2::XMLElement *element) {
		return sf::Color(element->IntAttribute("r"), element->IntAttribute("g"), element->IntAttribute("b"), element->IntAttribute("a"));
	}

	void parseEffects(Effects &effects, tinyxml2::XMLElement *element) {
		if (element) {
			for (tinyxml2::XMLElement *effectEl : element) {
				effects.effects[effectEl->Attribute("name")] = effectEl->Attribute("ref");
			}
		}
	}

	void parse(ParticleEffect &effect, tinyxml2::XMLElement *element, ParticleEffectOptions options) {
		tinyxml2::XMLElement * particleEl = element;

		if (particleEl) {
			sf::Vector2f spriteSize(0, 0);

			tinyxml2::XMLElement * psizeEl = element->FirstChildElement("psize");
			if (psizeEl)
				spriteSize = sf::Vector2f{(float)psizeEl->IntAttribute("x"), (float)psizeEl->IntAttribute("y")};

			if (particleEl->Attribute("lifetime"))
				effect.lifetime = particleEl->FloatAttribute("lifetime");
			else
				effect.lifetime = std::numeric_limits<float>::max();

			int max = particleEl->IntAttribute("max") + 1;
			int count = particleEl->IntAttribute("count");
			effect.particles = count;
			switch (partSysModes[particleEl->Attribute("type")]) {
			case ParticleSystemMode::Points:
				effect.particleSystem = new particles::PointParticleSystem(max);
				break;
			case ParticleSystemMode::Lines: {
				auto lineSystem = new particles::LineParticleSystem(max);
				lineSystem->setPoints(sf::Vector2f(0, 0), sf::Vector2f(particleEl->FloatAttribute("x"), particleEl->FloatAttribute("y")));
				effect.particleSystem = lineSystem;
			}
			break;
			case ParticleSystemMode::Texture:
				effect.particleSystem = new particles::TextureParticleSystem(max, &(options.texMgr->getRef(particleEl->Attribute("name"))), options.screenSize.x, options.screenSize.y);
				break;
			case ParticleSystemMode::Spritesheet: {
				auto spriteSystem = new particles::SpriteSheetParticleSystem(max, &(options.texMgr->getRef(particleEl->Attribute("name"))), options.screenSize.x, options.screenSize.y);

				auto texCoordGen = spriteSystem->addGenerator<particles::TexCoordsGenerator>();
				texCoordGen->texCoords = sf::IntRect(options.direction * spriteSize.x, 0, spriteSize.x, spriteSize.y);

				if (options.applyShader) {
					spriteSystem->applyShader = options.applyShader;
					spriteSystem->shader = options.shader;
					spriteSystem->shaderOptions = options.shaderOptions;
				}

				effect.particleSystem = spriteSystem;
			}
			break;
			case ParticleSystemMode::AnimatedSpritesheet: {
				auto spriteSystem = new particles::SpriteSheetParticleSystem(max, &(options.texMgr->getRef(particleEl->Attribute("name"))), options.screenSize.x, options.screenSize.y);

				auto texCoordGen = spriteSystem->addGenerator<particles::TexCoordsGenerator>();
				texCoordGen->texCoords = sf::IntRect(options.direction * spriteSize.x, 0, spriteSize.x, spriteSize.y);

				auto animationUpdater = spriteSystem->addUpdater<particles::AnimationUpdater>();

				tinyxml2::XMLElement * animEl = particleEl->FirstChildElement("animation");

				if (animEl) {
					float duration = animEl->FirstChildElement("duration")->IntAttribute("value") / 1000.0;
					tinyxml2::XMLElement * framesEl = animEl->FirstChildElement("frames");

					if (framesEl) {
						for (tinyxml2::XMLElement *frameEl : framesEl) {
							if (frameEl->Attribute("n")) {
								int frame = frameEl->IntAttribute("n");
								animationUpdater->frames.push_back(sf::IntRect(options.direction * spriteSize.x, frame * spriteSize.y, spriteSize.x, spriteSize.y));
							} else {
								int x = frameEl->IntAttribute("x");
								int y = frameEl->IntAttribute("y");
								animationUpdater->frames.push_back(sf::IntRect(x * spriteSize.x, y * spriteSize.y, spriteSize.x, spriteSize.y));
							}

						}
					}

					animationUpdater->frameTime = duration;// / animationUpdater->frames.size();
					animationUpdater->looped = animEl->BoolAttribute("loop");
				}

				if (options.applyShader) {
					spriteSystem->applyShader = options.applyShader;
					spriteSystem->shader = options.shader;
					spriteSystem->shaderOptions = options.shaderOptions;
				}

				effect.particleSystem = spriteSystem;

			}
			break;
			case ParticleSystemMode::Metaball: {
				// FIXME size == screen size
				auto metaball = new particles::MetaballParticleSystem(max, &(options.texMgr->getRef(particleEl->Attribute("name"))), options.screenSize.x, options.screenSize.y, &metaballShader);
				metaball->color = this->parseColor(particleEl);
				effect.particleSystem = metaball;
			}
			break;
			default:
				break;
			}

			effect.continuous = particleEl->BoolAttribute("continuous");
			effect.alwaysVisible = particleEl->BoolAttribute("always_visible");

			if (effect.continuous)
				effect.particleSystem->emitRate = (float)count; // Particles per second. Use emitRate <= (maxNumberParticles / averageParticleLifetime) for constant streams
			else
				effect.particleSystem->emitRate = 0.0;

//			effect.particleSystem->emitRate = particleEl->FloatAttribute("rate"); // Particles per second. Use emitRate <= (maxNumberParticles / averageParticleLifetime) for constant streams
//			std::cout << "RATE "<<effect.particleSystem->emitRate<< " "<< effect.continuous << std::endl;
			tinyxml2::XMLElement *spawnerEl = particleEl->FirstChildElement("spawner");

			switch (spawnModes[spawnerEl->Attribute("type")]) {
			case SpawnerMode::Point:
				effect.spawner = effect.particleSystem->addSpawner<particles::PointSpawner>();
				break;
			case SpawnerMode::Box: {
				sf::Vector2f size(spawnerEl->FloatAttribute("x"), spawnerEl->FloatAttribute("y"));
				auto boxSpawner = effect.particleSystem->addSpawner<particles::BoxSpawner>();
				boxSpawner->size = size;
				effect.spawner = boxSpawner;
			}
			break;
			case SpawnerMode::Circle: {
				sf::Vector2f radius(spawnerEl->FloatAttribute("x"), spawnerEl->FloatAttribute("y"));
				auto circleSpawner = effect.particleSystem->addSpawner<particles::CircleSpawner>();
				circleSpawner->radius = radius;
				effect.spawner = circleSpawner;

			}
			break;
			case SpawnerMode::Disk: {
				sf::Vector2f radius(spawnerEl->FloatAttribute("x"), spawnerEl->FloatAttribute("y"));
				auto diskSpawner = effect.particleSystem->addSpawner<particles::DiskSpawner>();
				diskSpawner->radius = radius;
				effect.spawner = diskSpawner;
			}
			break;
			default:
				break;
			}
			effect.spawner->center = sf::Vector2f(0, 0);

			tinyxml2::XMLElement *timeGenEl = particleEl->FirstChildElement("time_generator");

			auto timeGenerator = effect.particleSystem->addGenerator<particles::TimeGenerator>();
			timeGenerator->minTime = timeGenEl->FloatAttribute("min_time");
			timeGenerator->maxTime = timeGenEl->FloatAttribute("max_time");

			tinyxml2::XMLElement *sizeGenEl = particleEl->FirstChildElement("size_generator");

			if (sizeGenEl) {
				auto sizeGenerator = effect.particleSystem->addGenerator<particles::SizeGenerator>();
				sizeGenerator->minStartSize = sizeGenEl->FloatAttribute("min_start_size");
				sizeGenerator->maxStartSize = sizeGenEl->FloatAttribute("max_start_size");
				sizeGenerator->minEndSize = sizeGenEl->FloatAttribute("min_end_size");
				sizeGenerator->maxEndSize = sizeGenEl->FloatAttribute("max_end_size");
			}

			tinyxml2::XMLElement *velGenEl = particleEl->FirstChildElement("velocity_generator");

			switch (velGenModes[velGenEl->Attribute("type")]) {
			case VelocityGeneratorMode::Vector:
			{
				auto vectorGenerator = effect.particleSystem->addGenerator<particles::VectorVelocityGenerator>();
				vectorGenerator->minStartVel = sf::Vector2f(velGenEl->FloatAttribute("min_start_vel_x"), velGenEl->FloatAttribute("min_start_vel_y"));
				vectorGenerator->maxStartVel = sf::Vector2f(velGenEl->FloatAttribute("max_start_vel_x"), velGenEl->FloatAttribute("max_start_vel_y"));
			}
			break;
			case VelocityGeneratorMode::Angled:
			{
				auto velocityGenerator = effect.particleSystem->addGenerator<particles::AngledVelocityGenerator>();
				velocityGenerator->minAngle = velGenEl->FloatAttribute("min_angle");
				velocityGenerator->maxAngle = velGenEl->FloatAttribute("max_angle");
				velocityGenerator->minStartSpeed = velGenEl->FloatAttribute("min_start_speed");
				velocityGenerator->maxStartSpeed = velGenEl->FloatAttribute("max_start_speed");
			}
			break;
			case VelocityGeneratorMode::Aimed:
			{
				auto aimedGenerator = effect.particleSystem->addGenerator<particles::AimedVelocityGenerator>();
				aimedGenerator->goal = options.destPos;
				aimedGenerator->minStartSpeed = velGenEl->FloatAttribute("min_start_speed");
				aimedGenerator->maxStartSpeed = velGenEl->FloatAttribute("max_start_speed");
			}
			break;
			default:
				break;
			}

			tinyxml2::XMLElement *rotGenEl = particleEl->FirstChildElement("rotation_generator");
			if (rotGenEl) {
				auto rotationGenerator = effect.particleSystem->addGenerator<particles::RotationGenerator>();
				rotationGenerator->minStartAngle = rotGenEl->FloatAttribute("min_start_angle");
				rotationGenerator->maxStartAngle = rotGenEl->FloatAttribute("max_start_angle");
				rotationGenerator->minEndAngle = rotGenEl->FloatAttribute("min_end_angle");
				rotationGenerator->maxEndAngle = rotGenEl->FloatAttribute("max_end_angle");
			}

			tinyxml2::XMLElement *colGenEl = particleEl->FirstChildElement("color_generator");

			if (colGenEl) {
				auto colorGenerator = effect.particleSystem->addGenerator<particles::ColorGenerator>();
				colorGenerator->minStartCol = this->parseColor(colGenEl->FirstChildElement("min_start_col"));
				colorGenerator->maxStartCol = this->parseColor(colGenEl->FirstChildElement("max_start_col"));
				colorGenerator->minEndCol = this->parseColor(colGenEl->FirstChildElement("min_end_col"));
				colorGenerator->maxEndCol = this->parseColor(colGenEl->FirstChildElement("max_end_col"));
			}
			auto timeUpdater = effect.particleSystem->addUpdater<particles::TimeUpdater>();
			auto colorUpdater = effect.particleSystem->addUpdater<particles::ColorUpdater>();
			auto sizeUpdater = effect.particleSystem->addUpdater<particles::SizeUpdater>();
			auto rotationUpdater = effect.particleSystem->addUpdater<particles::RotationUpdater>();
			auto eulerUpdater = effect.particleSystem->addUpdater<particles::EulerUpdater>();
			tinyxml2::XMLElement *eulUpEl = particleEl->FirstChildElement("euler_updater");
			if (eulUpEl) {
				eulerUpdater->globalAcceleration = sf::Vector2f(eulUpEl->FloatAttribute("accel_x"), eulUpEl->FloatAttribute("accel_y"));
			}

			if (velGenModes[velGenEl->Attribute("type")] == VelocityGeneratorMode::Aimed) {
				auto destinationUpdater = effect.particleSystem->addUpdater<particles::DestinationUpdater>();
				destinationUpdater->destination = options.destPos;
				destinationUpdater->delta = 16.0;
			}
		}
	}
};