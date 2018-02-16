#pragma once

#include "Config.hpp"

#include "Components/Components.hpp"

#include "third_party/tinyxml2.h"
#include "third_party/tixml2ex.h"

class TileParser {
public:
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

			// offset not needed, use center rect instead
//			if (offsetEl)
//				tile.offset = sf::Vector2i{offsetEl->IntAttribute("x"), offsetEl->IntAttribute("y")};
//			else
			tile.offset = sf::Vector2i{0, 0};			
		}
	}
};


enum class SpritesheetType {
	Static,
	Animated,
};

static std::map<std::string, SpritesheetType> spritesheetTypes =
{
	{ "static", SpritesheetType::Static },
	{ "animated", SpritesheetType::Animated },
};


class SpritesheetsParser {
public:

	std::vector<sf::Vector2i> parseFrames(tinyxml2::XMLElement *animEl) {
		tinyxml2::XMLElement * framesEl = animEl->FirstChildElement("frames");

		std::vector<sf::Vector2i> frames;
		for (tinyxml2::XMLElement *frameEl : framesEl) {
			if (frameEl->Attribute("n")) {
				int frame = frameEl->IntAttribute("n");
				frames.push_back(sf::Vector2i(0, frame));
			} else {
				int x = frameEl->IntAttribute("x");
				int y = frameEl->IntAttribute("y");
				frames.push_back(sf::Vector2i(x, y));
			}
		}

		return frames;
	}

	void parseAnimatedSpritesheet(AnimatedSpritesheet &spritesheet, tinyxml2::XMLElement *element) {
		int count = 1;
		if (element->Attribute("count"))
			count = element->IntAttribute("count");

		std::string stateNm = element->Attribute("name");
		spritesheet.states[stateNm] = std::vector<AnimatedSpriteView>();

		for (tinyxml2::XMLElement *viewEl : element) {
			for (int i = 0; i < count; i++) {
				AnimatedSpriteView animView;

				if(viewEl->FirstChildElement("loop")) {
					animView.loop = viewEl->FirstChildElement("loop")->BoolAttribute("value");
				}

				animView.duration = (float)viewEl->FirstChildElement("duration")->IntAttribute("value") / 1000.0;

				for (sf::Vector2i p : this->parseFrames(viewEl)) {
					animView.frames.push_back(sf::Vector2i(p.x+i, p.y));
				}
				spritesheet.states[stateNm].push_back(animView);
			}
		}
	}

	void parseStaticSpritesheet(StaticSpritesheet &spritesheet, tinyxml2::XMLElement *element) {
		std::string stateNm = element->Attribute("name");

		for (tinyxml2::XMLElement *viewEl : element) {
			SpriteView view;
			view.currentPosition = sf::Vector2i(viewEl->IntAttribute("x"), viewEl->IntAttribute("y"));

			spritesheet.states[stateNm].push_back(view);
		}

	}

	bool parseAnimatedSpritesheets(AnimatedSpritesheet &spritesheet, tinyxml2::XMLElement *element) {
		int viewsCount = 0;
		tinyxml2::XMLElement * spritesheetsEl = element->FirstChildElement("spritesheets");

		for (tinyxml2::XMLElement *views : element) {
			if (spritesheetTypes[views->Attribute("type")] == SpritesheetType::Animated)
			{
				this->parseAnimatedSpritesheet(spritesheet, views);
				viewsCount++;
			}
		}

		return (viewsCount > 0);
	}

	bool parseStaticSpritesheets(StaticSpritesheet &spritesheet, tinyxml2::XMLElement *element) {
		int viewsCount = 0;
		tinyxml2::XMLElement * spritesheetsEl = element->FirstChildElement("spritesheets");

		for (tinyxml2::XMLElement *views : element) {
			if (spritesheetTypes[views->Attribute("type")] == SpritesheetType::Static)
			{
				this->parseStaticSpritesheet(spritesheet, views);
				viewsCount++;
			}
		}

		return (viewsCount > 0);
	}

};

class GameObjectParser {
public:
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
			if (soundsEl) {
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


class ResourceParser {
public:
	void parse(Resource &resource, tinyxml2::XMLElement *element) {
		if (element) {
			resource.level = element->FirstChildElement("level")->IntAttribute("value");
			if(element->FirstChildElement("max_level"))
				resource.maxLevel = element->FirstChildElement("max_level")->IntAttribute("value");
		}
	}
};

class DecorParser {
public:
	void parse(Decor &decor, tinyxml2::XMLElement *element) {
		if (element) {
			decor.blocking = element->FirstChildElement("blocking")->BoolAttribute("value");
		}
	}
};
