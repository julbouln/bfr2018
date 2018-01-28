#pragma once

#include "Config.hpp"

#include "Components.hpp"

#include "tinyxml2.h"
#include "tixml2ex.h"

enum class TextureLoadMode {
	Default,
	WithWhiteMask,
	Button,
	BuildButton,
	WithDirections
};

static std::map<std::string, TextureLoadMode> texLoadModes =
{
	{ "default", TextureLoadMode::Default },
	{ "with_white_mask", TextureLoadMode::WithWhiteMask },
	{ "button", TextureLoadMode::Button },
	{ "build_button", TextureLoadMode::BuildButton },
	{ "with_directions", TextureLoadMode::WithDirections },
};

class TextureLoader {
	TextureManager *texManager;
public:
	std::map<std::string, sf::IntRect> centerRects;

	void setManager(TextureManager *mgr) {
		this->texManager = mgr;
	}

	void getSpecialPix(std::string name, sf::Image &image, int width, int height) {
		sf::Vector2i pix1;
		sf::Vector2i pix2;
		int found = 0;
		for (int x = 0; x < width; x++) {
			for (int y = 0; y < height; y++) {
				if (found > 1)
					return;
				if (image.getPixel(x, y) == sf::Color(255, 36, 196)) {
#ifdef PARSER_DEBUG
					std::cout << "TextureLoader: " << name << " special pix " << x << "x" << y << std::endl;
#endif
					if (found == 0) {
						pix1 = sf::Vector2i(x, y);
					} else if (found == 1) {
						pix2 = sf::Vector2i(x, y);
						sf::IntRect centerRect(pix1, pix2 - pix1);
#ifdef PARSER_DEBUG
						std::cout << "TextureLoader: " << name << " center rect " << centerRect.left << "x" << centerRect.top << ":" << centerRect.width << "x" << centerRect.height << std::endl;
#endif
						centerRects[name] = centerRect;
					}

					found++;
				}
			}
		}

		if (found) {

		}
	}

	void loadTextureWithWhiteMask(std::string name, std::string filename) {
		sf::Image img;
		img.loadFromFile(filename);
		this->getSpecialPix(name, img, img.getSize().x, img.getSize().y);
		img.createMaskFromColor(sf::Color::White);
		texManager->loadTexture(name, img, sf::IntRect{0, 0, (int)img.getSize().x, (int)img.getSize().y});
	}

	void loadTextureWithDirections(std::string name, std::string imgPath) {
		sf::Image image, outImage;

		image.loadFromFile(imgPath);
		int columnWidth = image.getSize().x / 5;
		int height = image.getSize().y;

		this->getSpecialPix(name, image, columnWidth, height);

		outImage.create(columnWidth * 8, height, sf::Color::Transparent);

		std::vector<sf::Image> directionsImg;

		for (int i = 0; i < 5; i++) {
			sf::Image dirImg;
			dirImg.create(columnWidth, height, sf::Color::Transparent);
			dirImg.copy(image, 0, 0, sf::IntRect(i * columnWidth, 0, columnWidth, height), true);
			directionsImg.push_back(dirImg);
		}

		for (int i = 1; i < 4; i ++) {
			sf::Image dirImg;
			dirImg.create(columnWidth, height, sf::Color::Transparent);
			dirImg.copy(image, 0, 0, sf::IntRect(i * columnWidth, 0, columnWidth, height), true);
			dirImg.flipHorizontally();
			directionsImg.push_back(dirImg);
		}

		for (int i = 0; i < 8; i++) {
			outImage.copy(directionsImg[i], i * columnWidth, 0, sf::IntRect(0, 0, columnWidth, height), true);
		}

		outImage.createMaskFromColor(sf::Color::White);
#ifdef PARSER_DEBUG
		std::cout << "TextureLoader: init dir texture " << name << " " << columnWidth << "x" << height << std::endl;
#endif
		texManager->loadTexture(name, outImage, sf::IntRect {0, 0, columnWidth * 8, height});
	}

	void loadButton(std::string name, std::string filename) {
		sf::Image but;
		but.loadFromFile(filename);
		texManager->loadTexture(name, but, sf::IntRect{0, 0, (int)but.getSize().x, (int)but.getSize().y / 2});
		texManager->loadTexture(name + "_down", but, sf::IntRect{0, (int)(but.getSize().y / 2), (int)but.getSize().x, (int)(but.getSize().y) / 2});
	}

	void loadBuildButton(std::string name, std::string filename) {
		sf::Image but;
		but.loadFromFile(filename);
		int height = (int)but.getSize().y / 5;
		texManager->loadTexture(name, but, sf::IntRect{0, 0, (int)but.getSize().x, height});
		texManager->loadTexture(name + "_down", but, sf::IntRect{0, height, (int)but.getSize().x, height});
		texManager->loadTexture(name + "_building", but, sf::IntRect{0, height * 2, (int)but.getSize().x, height});
		texManager->loadTexture(name + "_built", but, sf::IntRect{0, height * 3, (int)but.getSize().x, height});
		texManager->loadTexture(name + "_built_down", but, sf::IntRect{0, height * 4, (int)but.getSize().x, height});
	}

	void recParse(std::string name, tinyxml2::XMLElement *element) {
		for (tinyxml2::XMLElement *child : element) {
			std::string cname = name;
			if (child->Attribute("name")) {
				cname = name + "_" + child->Attribute("name");
			}

//			std::cout << "TextureParser: recParse child " << child->Name() << std::endl;

			if (std::string(child->Name()) == "texture")
			{
				if (child->Attribute("path")) {
					std::string path = child->Attribute("path");
					TextureLoadMode mode = TextureLoadMode::Default;
					if (child->Attribute("mode"))
						mode = texLoadModes[child->Attribute("mode")];

					if (child->Attribute("global")) {
						cname = child->Attribute("global");
					}

#ifdef PARSER_DEBUG
					std::cout << "TextureLoader: recParse child " << cname << " " << path << " " << (int)mode << std::endl;
#endif

					switch (mode) {
					case TextureLoadMode::Default:
						texManager->loadTexture(cname, path);
						break;
					case TextureLoadMode::WithWhiteMask:
						this->loadTextureWithWhiteMask(cname, path);
						break;
					case TextureLoadMode::Button:
						this->loadButton(cname, path);
						break;
					case TextureLoadMode::BuildButton:
						this->loadBuildButton(cname, path);
						break;
					case TextureLoadMode::WithDirections:
						this->loadTextureWithDirections(cname, path);
						break;
					}
				}
			}

			this->recParse(cname, child);
		}
	}

	void parse(tinyxml2::XMLElement *element) {
		this->recParse(element->Attribute("name"), element);
	}

	std::string getName(tinyxml2::XMLElement *element) {
		return element->Attribute("name");
	}

	void parseFile(std::string filename) {
		tinyxml2::XMLDocument doc;
		doc.LoadFile(filename.c_str());
		this->parse(doc.RootElement());
	}

};

class SoundBufferLoader {
	SoundBufferManager *sndManager;
public:
	void setManager(SoundBufferManager *mgr) {
		this->sndManager = mgr;
	}

	void recParse(std::string name, tinyxml2::XMLElement *element) {
		int i = 0;
		int cnt = 0;
		for (tinyxml2::XMLElement *child : element) {
			std::string nodeName = child->Name();
			if (nodeName == "sound_buffer")
				cnt++;
		}

		if (cnt == 1) {
			tinyxml2::XMLElement *child = element->FirstChildElement("sound_buffer");
			std::string cname = name;
			if (child->Attribute("name")) {
				std::string cName = child->Attribute("name");
				cname = name + "_" + cName;
			}

			if (child->Attribute("global")) {
				cname = child->Attribute("global");
			}

			sndManager->loadSoundBuffer(cname, child->Attribute("path"));
		} else {
			for (tinyxml2::XMLElement *child : element) {
				std::string nodeName = child->Name();
				std::string cname = name;
				if (child->Attribute("name")) {
					std::string cName = child->Attribute("name");
					cname = name + "_" + cName;
				}

				if (nodeName == "sound_buffer")
				{
					std::string ncname = cname + "_" + std::to_string(i);
					if (child->Attribute("path")) {
						std::string path = child->Attribute("path");

#ifdef PARSER_DEBUG
						std::cout << "SoundBufferLoader: recParse child " << ncname << " " << path << std::endl;
#endif

						if (child->Attribute("global")) {
							ncname = child->Attribute("global");
						}

						sndManager->loadSoundBuffer(ncname, path);
					}
					i++;
				}
				this->recParse(cname, child);
			}

		}
	}
	void parse(tinyxml2::XMLElement *element) {
		this->recParse(element->Attribute("name"), element);
	}

	void parseFile(std::string filename) {
		tinyxml2::XMLDocument doc;
		doc.LoadFile(filename.c_str());
		this->parse(doc.RootElement());
	}

};

class TileParser {
public:
	Animation parseAnim(tinyxml2::XMLElement *animEl) {
		float duration = animEl->FirstChildElement("duration")->IntAttribute("value") / 1000.0;
		tinyxml2::XMLElement * framesEl = animEl->FirstChildElement("frames");

		std::vector<int> frames;
		for (tinyxml2::XMLElement *frameEl : framesEl) {
			int frame = frameEl->IntAttribute("n");
			frames.push_back(frame);
		}

		Animation anim(frames, duration);

#ifdef PARSER_DEBUG
		std::cout << "TileParser: create animation containing " << frames.size() << " frames" << std::endl;
#endif

		return anim;
	}

	void parse(Tile &tile, tinyxml2::XMLElement *element) {
		if (element) {
			tinyxml2::XMLElement * sizeEl = element->FirstChildElement("size");
			tinyxml2::XMLElement * psizeEl = element->FirstChildElement("psize");
			tinyxml2::XMLElement * offsetEl = element->FirstChildElement("offset");
			tinyxml2::XMLElement * zEl = element->FirstChildElement("z");
			tinyxml2::XMLElement * animsEl = element->FirstChildElement("animations");

			if (zEl)
				tile.z = zEl->IntAttribute("value");
			else
				tile.z = 0;

			if (sizeEl)
				tile.size = sf::Vector2i{sizeEl->IntAttribute("x"), sizeEl->IntAttribute("y")};
			else
				tile.size = sf::Vector2i{1, 1};

			tile.psize = sf::Vector2f{(float)psizeEl->IntAttribute("x"), (float)psizeEl->IntAttribute("y")};

			if (offsetEl)
				tile.offset = sf::Vector2i{offsetEl->IntAttribute("x"), offsetEl->IntAttribute("y")};
			else
				tile.offset = sf::Vector2i{0, 0};

			int directions = 1;
			if (element->Attribute("directions"))
				directions = element->IntAttribute("directions");

			if (animsEl) {
				for (tinyxml2::XMLElement *animEl : animsEl) {
					std::string stateNm = animEl->Attribute("name");

					AnimationHandler animHandler;

					animHandler.frameSize = sf::IntRect(0, 0, tile.psize.x, tile.psize.y);

					for (int i = 0; i < directions; i++) {
						Animation anim = this->parseAnim(animEl);
						animHandler.addAnim(anim);
					}
					animHandler.update(0.0f);

					tile.animHandlers[stateNm] = animHandler;
#ifdef PARSER_DEBUG
					std::cout << "TileParser: add animation handler " << stateNm << " containing " << directions << " directions" << std::endl;
#endif
				}
			}
		}
	}
};

class GameObjectParser {
public:
	std::map<std::string, std::string> parseEffects(tinyxml2::XMLElement *element) {
		std::map<std::string, std::string> effects;
		tinyxml2::XMLElement * effectsEl = element->FirstChildElement("effects");
		if (effectsEl) {
			for (tinyxml2::XMLElement *effectEl : effectsEl) {
				effects[effectEl->Attribute("name")] = effectEl->Attribute("ref");
			}
		}
		return effects;
	}

	void parse(GameObject &obj, tinyxml2::XMLElement *element) {
		obj.view = element->FirstChildElement("view")->IntAttribute("value");
		obj.life = (float)element->FirstChildElement("life")->IntAttribute("value");
		obj.maxLife = obj.life;
		obj.name = element->FirstChildElement("name")->Attribute("value");
		obj.team = element->FirstChildElement("team")->Attribute("value");

	}
};

class UnitParser {
public:
	void parse(Unit &unit, tinyxml2::XMLElement *element) {
		if (element) {

			unit.speed = element->FirstChildElement("speed")->IntAttribute("value");

			unit.attack1 = Attack{(unsigned int)element->FirstChildElement("attack1")->IntAttribute("power"), 0};
			unit.attack2 = Attack{(unsigned int)element->FirstChildElement("attack2")->IntAttribute("power"), (unsigned int)element->FirstChildElement("attack2")->IntAttribute("dist")};

			tinyxml2::XMLElement * soundsEl = element->FirstChildElement("sound_actions");

			for (tinyxml2::XMLElement *soundEl : soundsEl) {
				std::string stName = soundEl->Attribute("name");
				if (soundEl) {
					int i = 0;
					for (tinyxml2::XMLElement *bufEl : soundEl) {
						i++;
					}
					unit.soundActions[stName] = i;

				} else {
					unit.soundActions[stName] = 0;
				}
			}

			tinyxml2::XMLElement * soundAttackEl = element->FirstChildElement("sound_attack");
			if (soundAttackEl) {
				std::string sName = soundAttackEl->Attribute("name");
				unit.attackSound = sName;
			}
		}
	}
};

class BuildingParser {
public:
	void parse(Building &building, tinyxml2::XMLElement *element) {
		if (element) {
			building.buildTime = (float)element->FirstChildElement("build_time")->IntAttribute("value");
			building.maxBuildTime = building.buildTime;
		}
	}
};

enum class ParticleSystemMode {
	Points,
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

class ParticleEffectParser {
public:
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

	void parse(ParticleEffect &effect, tinyxml2::XMLElement *element, TextureManager &texManager) {
		if (element) {

			tinyxml2::XMLElement * particleEl = element;

			if (particleEl) {
				int max = particleEl->IntAttribute("max");
				int count = particleEl->IntAttribute("count");
				effect.particles = count;
				switch (partSysModes[particleEl->Attribute("type")]) {
				case ParticleSystemMode::Points:
					effect.particleSystem = new particles::PointParticleSystem(max);
					break;
				case ParticleSystemMode::Texture:
					effect.particleSystem = new particles::TextureParticleSystem(max, &(texManager.getRef(particleEl->Attribute("name"))));
					break;
				case ParticleSystemMode::Spritesheet:
				case ParticleSystemMode::AnimatedSpritesheet:
					effect.particleSystem = new particles::SpriteSheetParticleSystem(max, &(texManager.getRef(particleEl->Attribute("name"))));
					break;
				default:
					// TODO
					break;
				}

				effect.particleSystem->emitRate = particleEl->FloatAttribute("rate"); // Particles per second. Use emitRate <= (maxNumberParticles / averageParticleLifetime) for constant streams

				tinyxml2::XMLElement *spawnerEl = particleEl->FirstChildElement("spawner");
				switch (spawnModes[spawnerEl->Attribute("type")]) {
				case SpawnerMode::Point:
					effect.spawner = effect.particleSystem->addSpawner<particles::PointSpawner>();
					break;
				default:
					// TODO
					break;
				}
				effect.spawner->center = sf::Vector2f(0, 0);

				tinyxml2::XMLElement *timeGenEl = particleEl->FirstChildElement("time_generator");

				auto timeGenerator = effect.particleSystem->addGenerator<particles::TimeGenerator>();
				timeGenerator->minTime = timeGenEl->FloatAttribute("min_time");
				timeGenerator->maxTime = timeGenEl->FloatAttribute("max_time");

				tinyxml2::XMLElement *sizeGenEl = particleEl->FirstChildElement("size_generator");

				auto sizeGenerator = effect.particleSystem->addGenerator<particles::SizeGenerator>();
				sizeGenerator->minStartSize = sizeGenEl->FloatAttribute("min_start_size");
				sizeGenerator->maxStartSize = sizeGenEl->FloatAttribute("max_start_size");
				sizeGenerator->minEndSize = sizeGenEl->FloatAttribute("min_end_size");
				sizeGenerator->maxEndSize = sizeGenEl->FloatAttribute("max_end_size");

				tinyxml2::XMLElement *velGenEl = particleEl->FirstChildElement("velocity_generator");

				switch (velGenModes[velGenEl->Attribute("type")]) {
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
//					aimedGenerator->goal = sf::Vector2f(0.5f * this->game->width, 0.5f * this->game->height);
					aimedGenerator->minStartSpeed = velGenEl->FloatAttribute("min_start_speed");
					aimedGenerator->maxStartSpeed = velGenEl->FloatAttribute("max_start_speed");
				}
				break;
				default:
					// TODO
					break;
				}

				tinyxml2::XMLElement *colGenEl = particleEl->FirstChildElement("color_generator");

				auto colorGenerator = effect.particleSystem->addGenerator<particles::ColorGenerator>();
				colorGenerator->minStartCol = this->parseColor(colGenEl->FirstChildElement("min_start_col"));
				colorGenerator->maxStartCol = this->parseColor(colGenEl->FirstChildElement("max_start_col"));
				colorGenerator->minEndCol = this->parseColor(colGenEl->FirstChildElement("min_end_col"));
				colorGenerator->maxEndCol = this->parseColor(colGenEl->FirstChildElement("max_end_col"));

				auto timeUpdater = effect.particleSystem->addUpdater<particles::TimeUpdater>();
				auto colorUpdater = effect.particleSystem->addUpdater<particles::ColorUpdater>();
				auto sizeUpdater = effect.particleSystem->addUpdater<particles::SizeUpdater>();
				auto rotationUpdater = effect.particleSystem->addUpdater<particles::RotationUpdater>();
				auto eulerUpdater = effect.particleSystem->addUpdater<particles::EulerUpdater>();
			}
		}
	}

};