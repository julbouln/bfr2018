#pragma once

#include "Config.hpp"

#include "Components/Components.hpp"

#include "Managers/TextureManager.hpp"
#include "Managers/SoundBufferManager.hpp"
#include "Managers/FontManager.hpp"
#include "Managers/ShaderManager.hpp"

#include "third_party/tinyxml2.h"
#include "third_party/tixml2ex.h"

#include "XmlParsers/ParticleEffectParser.hpp"
#include "XmlParsers/XmlLoaders.hpp"
#include "XmlParsers/XmlParser.hpp"

enum class TechComponent {
	Building,
	Character,
	Resource
};

class TechNode {
public:
	TechComponent comp;
	std::string type;
	std::vector<TechNode> children;
	std::string parentType;

	void parse(tinyxml2::XMLElement *el) {
		std::string comName = el->Name();
		if (comName == "building")
			comp = TechComponent::Building;
		if (comName == "char")
			comp = TechComponent::Character;
		if (comName == "resource")
			comp = TechComponent::Resource;

		type = el->Attribute("type");

#ifdef TECH_TREE_DEBUG
		std::cout << "TechTree: parse " << el->Name() << " " << type << " " << (int)comp << std::endl;
#endif

		for (tinyxml2::XMLElement *childEl : el) {
			TechNode childNode;
			childNode.parentType = this->type;
			childNode.parse(childEl);
			children.push_back(childNode);
		}
	}
};

class EntityFactory {
	bool loaded;

	std::map<std::string, tinyxml2::XMLDocument *> loadedXmlDocs;

	std::map<std::string, int> groupCount;
	std::map<std::string, std::vector<std::string>> groups;

	std::map<std::string, TechNode> techTrees;

	std::map<std::string, int> resourcesCount;

	std::map<int, std::vector<sf::Color> > playerColors;

public:
	TextureManager texManager;
	SoundBufferManager sndManager;
	FontManager fntManager;
	ShaderManager shrManager;

	TextureLoader texLoader;
	SoundBufferLoader sndLoader;

	TileParser tileParser;
	GameObjectParser gameObjectParser;
	UnitParser unitParser;
	BuildingParser buildingParser;
	ParticleEffectParser particleEffectParser;
	ResourceParser resourceParser;
	DecorParser decorParser;
	SpritesheetsParser spritesheetsParser;

	std::map<std::string, int> decorGenerator;

	sf::Texture &getTex(std::string name);
	sf::SoundBuffer &getSndBuf(std::string name);

	std::vector<TechNode *> getTechNodes(std::string team);
	TechNode *getTechNode(std::string team, std::string type);
	TechNode *getTechRoot(std::string team);

	sf::IntRect getCenterRect(std::string name);

	sf::Color getPlayerColor(sf::Color key, int idx);

	void setColorSwapShader(entt::Registry<EntityID> &registry, Tile &tile, EntityID playerEnt);
// XML loader

	void parseTileFromXml(std::string name, Tile &tile);
	void parseBuildingFromXml(std::string name, Building &building);
	void parseUnitFromXml(std::string name, Unit &unit);
	void parseGameObjectFromXml(std::string name, GameObject &obj);
	void parseResourceFromXml(std::string name, Resource &resource);
	void parseDecorFromXml(std::string name, Decor &decor);

	float buildTime(std::string type);
	float trainCost(std::string type);
	float objTypeLife(std::string type); // initial life

// Creator

	void destroyEntity(entt::Registry<EntityID> &registry, EntityID entity);

// Unit
	EntityID createUnit(entt::Registry<EntityID> &registry, EntityID playerEnt, std::string name, int x, int y);

// Building
	EntityID startBuilding(entt::Registry<EntityID> &registry, std::string name, EntityID constructedBy);
	EntityID finishBuilding(entt::Registry<EntityID> &registry, EntityID entity, EntityID playerEnt, int x, int y, bool built);
	bool placeBuilding(entt::Registry<EntityID> &registry, EntityID entity);

	EntityID plantResource(entt::Registry<EntityID> &registry, std::string name, int x, int y);
	EntityID growedResource(entt::Registry<EntityID> &registry, std::string name, EntityID entity);
	EntityID createParticleEffect(entt::Registry<EntityID> &registry, std::string name, ParticleEffectOptions options);
	EntityID createDecor(entt::Registry<EntityID> &registry, std::string name, int x, int y);

// Player
	EntityID createPlayer(entt::Registry<EntityID> &registry, std::string team, bool ai);

	void loadInitial();
	void load();

	EntityFactory();

private:
	void loadMisc();
	void autoTransition(sf::Image &img);
	void loadTerrains();
	void loadDecorGenerator(std::string filename);

	TechNode loadTechTree(std::string filename);
	void loadTechTrees();
	TechNode *recGetTechNodeByName(TechNode *node, std::string type);
	std::vector<TechNode *> recGetTechNodes(TechNode *node, std::vector<TechNode *> currentNodes);

	void loadPlayerColors(std::string filename);
	tinyxml2::XMLElement *getXmlComponent(std::string name, const char* component);
	std::string randGroupName(std::string name);
	void addStaticVerticalSpriteView(std::vector<SpriteView> &states, std::initializer_list<int> frames);
	void assignSpritesheets(entt::Registry<EntityID> &registry, EntityID entity, std::string name);

	void setDefaultSpritesheet(StaticSpritesheet& spritesheet, Tile &tile, int y);

	sf::Vector2f caseToPixel(sf::Vector2i pos);
	void loadManifest(std::string filename);

};