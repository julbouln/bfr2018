#pragma once

#include "Config.hpp"

#include "Components.hpp"
#include "TextureManager.hpp"
#include "SoundBufferManager.hpp"

#include "tinyxml2.h"
#include "tixml2ex.h"

#include "XmlParser.hpp"

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

	std::map<std::string, tinyxml2::XMLDocument *> docs;
	std::map<std::string, tinyxml2::XMLDocument *> loadedXmlDocs;

	std::map<std::string, TechNode> techTrees;

	std::map<std::string, int> resourcesCount;

	std::map<int, std::vector<sf::Color> > playerColors;

public:
	TextureManager texManager;
	SoundBufferManager sndManager;

	TextureLoader texLoader;
	SoundBufferLoader sndLoader;

	TileParser tileParser;
	GameObjectParser gameObjectParser;
	UnitParser unitParser;
	BuildingParser buildingParser;

	sf::Texture &getTex(std::string name) {
		return texManager.getRef(name);
	}

	sf::SoundBuffer &getSndBuf(std::string name) {
		return sndManager.getRef(name);
	}

	void loadInitial() {
		texLoader.loadTextureWithWhiteMask("intro_background", "medias/interface/bgs/toile.png");

//		texLoader.parseFile("defs/new/uni/zork.xml");
//		sndLoader.parseFile("defs/new/uni/zork.xml");

//		this->loadManifest2("defs/new/manifest.xml");
	}

	void loadMisc() {

		texLoader.loadTextureWithWhiteMask("interface_rebel", "medias/interface/bgs/interface_rebel_800x600.png");
		texLoader.loadTextureWithWhiteMask("interface_neonaz", "medias/interface/bgs/interface_neonaz_800x600.png");

		texLoader.loadTextureWithWhiteMask("indice_bg_rebel", "medias/interface/bgs/indice_bio-bg.png");
		texLoader.loadTextureWithWhiteMask("indice_bg_neonaz", "medias/interface/bgs/indice_pol-bg.png");

		texLoader.loadTextureWithWhiteMask("indice_rebel", "medias/interface/bgs/indice_bio.png");
		texLoader.loadTextureWithWhiteMask("indice_neonaz", "medias/interface/bgs/indice_pol.png");

		texLoader.loadButton("rebel_move", "medias/interface/buttons/rebel_move_button.png");
		texLoader.loadButton("rebel_attack", "medias/interface/buttons/rebel_attack_button.png");
		texLoader.loadButton("rebel_cancel", "medias/interface/buttons/annuler_rebelle.png");

		texLoader.loadButton("neonaz_move", "medias/interface/buttons/naz_move_button.png");
		texLoader.loadButton("neonaz_attack", "medias/interface/buttons/naz_attack_button.png");
		texLoader.loadButton("neonaz_cancel", "medias/interface/buttons/annuler_naz.png");

		texLoader.loadBuildButton("nature_icon", "medias/resources/nature-icon.png");
		texLoader.loadBuildButton("pollution_icon", "medias/resources/pollution-icon.png");

		texLoader.loadTextureWithWhiteMask("button2", "medias/interface/buttons/button2.png");

		texLoader.loadButton("menu_button", "medias/interface/buttons/menu_button.png");

		texLoader.loadTextureWithWhiteMask("shadow", "medias/misc/shadow.png");
		texLoader.loadTextureWithWhiteMask("selected", "medias/tiles/cadre_unit.png");
		texLoader.loadTextureWithWhiteMask("forbid", "medias/misc/forbide.png");

		texLoader.loadTextureWithWhiteMask("ruin", "medias/misc/ruine.png");

		texLoader.loadTextureWithWhiteMask("blood", "medias/misc/blood.png");

		sndManager.loadSoundBuffer("combo", "medias/misc/combo.wav");
		sndManager.loadSoundBuffer("killer", "medias/misc/killer.wav");
		sndManager.loadSoundBuffer("megakill", "medias/misc/megakill.wav");
		sndManager.loadSoundBuffer("barbarian", "medias/misc/barbarian.wav");
		sndManager.loadSoundBuffer("butchery", "medias/misc/butchery.wav");
	}

	void autoTransition(sf::Image &img) {
		// TODO: use an XML to let modifiying PNG for better transitions
		for (int col = 0; col < 5; col++) {
			img.copy(img, col * 32, 6 * 32, sf::IntRect(col * 32, 2 * 32, 32, 32), true);
			img.copy(img, col * 32, 6 * 32, sf::IntRect(col * 32, 4 * 32, 32, 32), true);

			img.copy(img, col * 32, 7 * 32, sf::IntRect(col * 32, 1 * 32, 32, 32), true);
			img.copy(img, col * 32, 7 * 32, sf::IntRect(col * 32, 2 * 32, 32, 32), true);
			img.copy(img, col * 32, 7 * 32, sf::IntRect(col * 32, 4 * 32, 32, 32), true);

			img.copy(img, col * 32, 9 * 32, sf::IntRect(col * 32, 1 * 32, 32, 32), true);
			img.copy(img, col * 32, 9 * 32, sf::IntRect(col * 32, 8 * 32, 32, 32), true);

			img.copy(img, col * 32, 11 * 32, sf::IntRect(col * 32, 1 * 32, 32, 32), true);
			img.copy(img, col * 32, 11 * 32, sf::IntRect(col * 32, 2 * 32, 32, 32), true);
			img.copy(img, col * 32, 11 * 32, sf::IntRect(col * 32, 8 * 32, 32, 32), true);

			img.copy(img, col * 32, 13 * 32, sf::IntRect(col * 32, 1 * 32, 32, 32), true);
			img.copy(img, col * 32, 13 * 32, sf::IntRect(col * 32, 4 * 32, 32, 32), true);
			img.copy(img, col * 32, 13 * 32, sf::IntRect(col * 32, 8 * 32, 32, 32), true);

			img.copy(img, col * 32, 14 * 32, sf::IntRect(col * 32, 2 * 32, 32, 32), true);
			img.copy(img, col * 32, 14 * 32, sf::IntRect(col * 32, 4 * 32, 32, 32), true);
			img.copy(img, col * 32, 14 * 32, sf::IntRect(col * 32, 8 * 32, 32, 32), true);

			img.copy(img, col * 32, 15 * 32, sf::IntRect(col * 32, 1 * 32, 32, 32), true);
			img.copy(img, col * 32, 15 * 32, sf::IntRect(col * 32, 2 * 32, 32, 32), true);
			img.copy(img, col * 32, 15 * 32, sf::IntRect(col * 32, 4 * 32, 32, 32), true);
			img.copy(img, col * 32, 15 * 32, sf::IntRect(col * 32, 8 * 32, 32, 32), true);

			img.copy(img, col * 32, (16 + 3) * 32, sf::IntRect(col * 32, (16 + 1) * 32, 32, 32), true);
			img.copy(img, col * 32, (16 + 3) * 32, sf::IntRect(col * 32, (16 + 2) * 32, 32, 32), true);

			img.copy(img, col * 32, (16 + 5) * 32, sf::IntRect(col * 32, (16 + 1) * 32, 32, 32), true);
			img.copy(img, col * 32, (16 + 5) * 32, sf::IntRect(col * 32, (16 + 4) * 32, 32, 32), true);

			img.copy(img, col * 32, (16 + 10) * 32, sf::IntRect(col * 32, (16 + 2) * 32, 32, 32), true);
			img.copy(img, col * 32, (16 + 10) * 32, sf::IntRect(col * 32, (16 + 8) * 32, 32, 32), true);

			img.copy(img, col * 32, (16 + 12) * 32, sf::IntRect(col * 32, (16 + 4) * 32, 32, 32), true);
			img.copy(img, col * 32, (16 + 12) * 32, sf::IntRect(col * 32, (16 + 8) * 32, 32, 32), true);

			img.copy(img, col * 32, (16 + 13) * 32, sf::IntRect(col * 32, (16 + 1) * 32, 32, 32), true);
			img.copy(img, col * 32, (16 + 13) * 32, sf::IntRect(col * 32, (16 + 4) * 32, 32, 32), true);
			img.copy(img, col * 32, (16 + 13) * 32, sf::IntRect(col * 32, (16 + 8) * 32, 32, 32), true);

			img.copy(img, col * 32, (16 + 14) * 32, sf::IntRect(col * 32, (16 + 2) * 32, 32, 32), true);
			img.copy(img, col * 32, (16 + 14) * 32, sf::IntRect(col * 32, (16 + 4) * 32, 32, 32), true);
			img.copy(img, col * 32, (16 + 14) * 32, sf::IntRect(col * 32, (16 + 8) * 32, 32, 32), true);

			img.copy(img, col * 32, (16 + 15) * 32, sf::IntRect(col * 32, (16 + 1) * 32, 32, 32), true);
			img.copy(img, col * 32, (16 + 15) * 32, sf::IntRect(col * 32, (16 + 2) * 32, 32, 32), true);
			img.copy(img, col * 32, (16 + 15) * 32, sf::IntRect(col * 32, (16 + 4) * 32, 32, 32), true);
			img.copy(img, col * 32, (16 + 15) * 32, sf::IntRect(col * 32, (16 + 8) * 32, 32, 32), true);
		}
	}

	void loadTerrains() {
		sf::Image terrains;
		terrains.loadFromFile("medias/tiles/terrains.png");
		texManager.loadTexture("sand", terrains, sf::IntRect{0, 0, 32, 96});
		texManager.loadTexture("water", terrains, sf::IntRect{32, 0, 32, 96});
		texManager.loadTexture("grass", terrains, sf::IntRect{64, 0, 32, 96});
		texManager.loadTexture("dirt", terrains, sf::IntRect{96, 0, 32, 96});
		texManager.loadTexture("concrete", terrains, sf::IntRect{128, 0, 32, 96});

		sf::Image transitions;
		transitions.loadFromFile("medias/new/transitions.png");
		transitions.createMaskFromColor(sf::Color::White);

		this->autoTransition(transitions);

		texManager.loadTexture("sand_transition", transitions, sf::IntRect{0, 0, 32, 1024});
		texManager.loadTexture("water_transition", transitions, sf::IntRect{32, 0, 32, 1024});
		texManager.loadTexture("dirt_transition", transitions, sf::IntRect{96, 0, 32, 1024});
		texManager.loadTexture("concrete_transition", transitions, sf::IntRect{128, 0, 32, 1024});

		texManager.loadTexture("fog_transition", "medias/new/fog.png");
		texManager.loadTexture("debug_transition", "medias/new/debug_transitions256.png");
	}

	TechNode loadTechTree(std::string filename) {
		tinyxml2::XMLDocument doc;
		doc.LoadFile(filename.c_str());
		TechNode tech;
		tech.parse(doc.RootElement()->FirstChildElement());

		return tech;
	}

	void loadTechTrees() {
		techTrees["rebel"] = this->loadTechTree("defs/tech/rebels.xml");
		techTrees["neonaz"] = this->loadTechTree("defs/tech/neonaz.xml");
	}

	TechNode *recGetTechNodeByName(TechNode *node, std::string type) {
		if (node->type == type)
			return node;
		else
			for (TechNode &child : node->children) {
				TechNode *cnode = recGetTechNodeByName(&child, type);
				if (cnode)
					return cnode;
			}
		return nullptr;
	}

	std::vector<TechNode *> recGetTechNodes(TechNode *node, std::vector<TechNode *> currentNodes) {
		std::vector<TechNode *>nodes = currentNodes;
		for (TechNode &child : node->children) {
			nodes.push_back(&child);
			nodes = recGetTechNodes(&child, nodes);
		}
		return nodes;
	}

	std::vector<TechNode *> getTechNodes(std::string team) {
		std::vector<TechNode *> nodes;
		nodes = this->recGetTechNodes(&techTrees[team], nodes);
		return nodes;
	}

	TechNode *getTechNode(std::string team, std::string type) {
		return this->recGetTechNodeByName(&this->techTrees[team], type);
	}

	TechNode *getTechRoot(std::string team) {
		return &this->techTrees[team];
	}

	sf::IntRect getCenterRect(std::string name) {
		return texLoader.centerRects[name];
	}

	sf::Color getPlayerColor(sf::Color key, int idx) {
		return playerColors[key.r << 16 + key.g << 8 + key.b][idx];
	}

// XML loader

	void loadResources(std::string filename) {
		tinyxml2::XMLDocument *doc = new tinyxml2::XMLDocument();
		doc->LoadFile(filename.c_str());
		std::string name = doc->RootElement()->Attribute("name");
		this->docs[name] = doc;

		texLoader.loadTextureWithWhiteMask(name, "medias/resources/" + name + ".png");

		int i = 1;
		for (tinyxml2::XMLElement *el : doc->RootElement()) {

			std::string imgfile = el->FirstChildElement("file")->Attribute("path");
			texLoader.loadTextureWithWhiteMask(name + std::to_string(i), imgfile);
			i++;
		}
		resourcesCount[name] = i - 1;
	}

	void loadPlayerColors(std::string filename) {
		tinyxml2::XMLDocument *doc = new tinyxml2::XMLDocument();
		doc->LoadFile(filename.c_str());

		for (tinyxml2::XMLElement *vcolEl : doc->RootElement()) {
			sf::Color key = sf::Color(vcolEl->IntAttribute("r"), vcolEl->IntAttribute("g"), vcolEl->IntAttribute("b"));
			std::vector<sf::Color> colors;
			for (tinyxml2::XMLElement *colEl : vcolEl) {
				colors.push_back(sf::Color(colEl->IntAttribute("r"), colEl->IntAttribute("g"), colEl->IntAttribute("b")));
			}
			playerColors[key.r << 16 + key.g << 8 + key.b] = colors;
		}
	}

	tinyxml2::XMLElement *getXmlComponent(std::string name, const char* component) {
		if (this->loadedXmlDocs.count(name) > 0)
			return this->loadedXmlDocs[name]->RootElement()->FirstChildElement(component);
		else
			return nullptr;
	}

	void parseTileFromXml(std::string name, Tile &tile) {
		tileParser.parse(tile, this->getXmlComponent(name, "tile"));
	}

	void parseGameObjectFromXml(std::string name, GameObject &obj) {
		gameObjectParser.parse(obj, this->getXmlComponent(name, "game_object"));
	}

// Creator

	void destroyEntity(entt::Registry<EntityID> &registry, EntityID entity) {
#ifdef FACTORY_DEBUG
		std::cout << "EntityFactory: destroy " << entity << std::endl;
#endif
		registry.destroy(entity);
	}

// Terrain
	EntityID createTerrain(entt::Registry<EntityID> &registry, std::string name, int variant) {
		EntityID entity = registry.create();

#ifdef FACTORY_DEBUG
		std::cout << "EntityFactory: create terrain " << entity << " " << name << " " << variant << std::endl;
#endif

		Tile tile;
		tile.psize = sf::Vector2f{32, 32};
		tile.size = sf::Vector2i{1, 1};

		tile.pos = sf::Vector2i(0, 0);
		tile.ppos = sf::Vector2f(tile.pos) * (float)32.0;
		tile.z = 0;

		tile.sprite.setTexture(texManager.getRef(name));

		tile.centerRect = sf::IntRect(0, 0, 32, 32);

		Animation staticAnim({0, 1, 2});

		AnimationHandler idleHandler;

		idleHandler.frameSize = sf::IntRect(0, 0, 32, 32);

		idleHandler.addAnim(staticAnim);

		idleHandler.changeColumn(0);
		idleHandler.set(variant);

		tile.sprite.setTextureRect(idleHandler.bounds); // texture need to be updated

		tile.animHandlers["idle"] = idleHandler;

		tile.direction = North;
		tile.state = "idle";

		registry.assign<Tile>(entity, tile);
		return entity;
	}

// Unit
#define UNIT_FRAME_COUNT 10

	EntityID createUnit(entt::Registry<EntityID> &registry, EntityID player, std::string name, int x, int y) {
		EntityID entity = registry.create();
#ifdef FACTORY_DEBUG
		std::cout << "EntityFactory: create unit " << entity << " " << name << " at " << x << "x" << y << std::endl;
#endif
		Tile tile;
//		this->parseTileFromXml(name, tile, 8);
		tileParser.parse(tile, this->getXmlComponent(name, "tile"));

		tile.pos = sf::Vector2i(x, y);
		tile.ppos = sf::Vector2f(tile.pos) * (float)32.0;
		tile.z = 0;

		tile.sprite.setTexture(texManager.getRef(name));

		tile.direction = South;
		tile.state = "idle";
		tile.sprite.setTextureRect(tile.animHandlers["idle"].bounds); // texture need to be updated

		tile.centerRect = this->getCenterRect(name);

		GameObject obj;
//		this->parseGameObjectFromXml(name, obj);
		gameObjectParser.parse(obj, this->getXmlComponent(name, "game_object"));

		for (auto o : gameObjectParser.parseEffects(this->getXmlComponent(name, "game_object"))) {
#ifdef FACTORY_DEBUG
			std::cout << "EntityFactory: " << entity << " add effect " << o.first << " from ref " << o.second << std::endl;
#endif
			obj.effects[o.first] = this->createMapEffect(registry, o.second);
		}

//		this->addProjectileFromXml(registry, name, obj );
		obj.player = player;
		obj.mapped = true;
		obj.destroy = false;

		Unit unit;
		unitParser.parse(unit, this->getXmlComponent(name, "unit"));
//		this->parseUnitFromXml(name, unit);

		unit.nextpos = tile.pos;
		unit.destAttack = 0;
		unit.nopath = 0;
		unit.destpos = tile.pos;

		registry.assign<Tile>(entity, tile);
		registry.assign<GameObject>(entity, obj);
		registry.assign<Unit>(entity, unit);

		return entity;
	}

// Building
	EntityID startBuilding(entt::Registry<EntityID> &registry, std::string name, EntityID constructedBy) {
		EntityID entity = registry.create();
#ifdef FACTORY_DEBUG
		std::cout << "EntityFactory: start building " << entity << " " << name << " constructed by " << constructedBy << std::endl;
#endif
		Building building;
//		this->parseBuildingFromXml(name, building);
		buildingParser.parse(building, this->getXmlComponent(name, "building"));
		building.construction = 0;
		building.constructedBy = constructedBy;

		GameObject obj;
//		this->parseGameObjectFromXml(name, obj);
		gameObjectParser.parse(obj, this->getXmlComponent(name, "game_object"));
		obj.player = 0;
		obj.mapped = false;
		obj.destroy = false;
		obj.life = obj.life * 4;
		obj.maxLife = obj.life;

		registry.assign<GameObject>(entity, obj);
		registry.assign<Building>(entity, building);
		return entity;
	}

	EntityID finishBuilding(entt::Registry<EntityID> &registry, EntityID entity, EntityID player, int x, int y, bool built) {
		GameObject &obj = registry.get<GameObject>(entity);
#ifdef FACTORY_DEBUG
		std::cout << "EntityFactory: finish building " << entity << " " << obj.name << " at " << x << "x" << y << std::endl;
#endif

		obj.player = player;
		obj.mapped = built;

//		obj.effects["explosion"] = this->createExplosionEffect(registry);

		Tile tile;
//		this->parseTileFromXml(obj.name, tile, 8);
		tileParser.parse(tile, this->getXmlComponent(obj.name, "tile"));

		tile.pos = sf::Vector2i(x, y);
		tile.ppos = sf::Vector2f(tile.pos) * (float)32.0;
		tile.z = 0;

		tile.sprite.setTexture(texManager.getRef(obj.name));

		tile.direction = North;
		tile.state = "idle";
		tile.sprite.setTextureRect(tile.animHandlers["idle"].bounds); // texture need to be updated

		tile.centerRect = this->getCenterRect(obj.name);

		registry.assign<Tile>(entity, tile);

		for (auto o : gameObjectParser.parseEffects(this->getXmlComponent(obj.name, "game_object"))) {
#ifdef FACTORY_DEBUG
			std::cout << "EntityFactory: " << entity << " add effect " << o.first << " from ref " << o.second << std::endl;
#endif
			obj.effects[o.first] = this->createMapEffect(registry, o.second);
		}

		return entity;
	}

	bool placeBuilding(entt::Registry<EntityID> &registry, EntityID entity) {
		Building &building = registry.get<Building>(entity);
		GameObject &obj = registry.get<GameObject>(entity);
		obj.mapped = true;
		if (building.constructedBy) {
			Building &buildingBy = registry.get<Building>(building.constructedBy);
			buildingBy.construction = 0;
			return true;
		} else {
			return false;
		}
	}

// Resource
	std::string resourceTypeName( ResourceType type) {
		switch (type) {
		case ResourceType::Nature:
			return "nature";
		case ResourceType::Pollution:
			return "pollution";
		}
	}

	EntityID plantResource(entt::Registry<EntityID> &registry, ResourceType type, int x, int y) {
		EntityID entity = registry.create();

#ifdef FACTORY_DEBUG
		std::cout << "EntityFactory: plant resource " << entity << " " << (int)type << " at " << x << "x" << y << std::endl;
#endif

		std::string name = this->resourceTypeName(type);
		Tile tile;
		tile.psize = sf::Vector2f{32, 32};
		tile.size = sf::Vector2i{1, 1};
		tile.z = 0;

		tile.pos = sf::Vector2i(x, y);
		tile.ppos = sf::Vector2f(tile.pos) * (float)32.0;

		tile.sprite.setTexture(texManager.getRef(name));

		Animation staticAnim({0});

		AnimationHandler idleHandler;

		idleHandler.frameSize = sf::IntRect(0, 0, 32, 32);

		idleHandler.addAnim(staticAnim);
		idleHandler.update(0.0f);

		tile.animHandlers["idle"] = idleHandler;

		tile.direction = North;
		tile.state = "idle";

		tile.centerRect = this->getCenterRect(name);

		Resource resource;
		resource.type = type;
		resource.level = 0;
		resource.grow = 0.0;

		registry.assign<Tile>(entity, tile);
		registry.assign<Resource>(entity, resource);

		return entity;
	}

	EntityID growedResource(entt::Registry<EntityID> &registry, std::string name, EntityID entity) {
		int rnd = rand() % resourcesCount[name];
		std::string rname = name + std::to_string(rnd + 1);
		Tile &oldTile = registry.get<Tile>(entity);

		Tile tile;

		tinyxml2::XMLDocument *doc = this->docs[name];
		int i = 0;
		for (tinyxml2::XMLElement *el : doc->RootElement()) {
			if (rnd == i) {
				tinyxml2::XMLElement * sizeEl = el->FirstChildElement("case_size");
				tinyxml2::XMLElement * offsetEl = el->FirstChildElement("decal_value");
				tinyxml2::XMLElement * psizeEl = el->FirstChildElement("pixel_size");

				tile.size = sf::Vector2i{sizeEl->IntAttribute("w"), sizeEl->IntAttribute("h")};
				tile.psize = sf::Vector2f{(float)psizeEl->IntAttribute("w"), (float)psizeEl->IntAttribute("h")};
				if (offsetEl) {
					tile.offset = sf::Vector2i{0, offsetEl->IntAttribute("h")};
				}
				else
					tile.offset = sf::Vector2i{0, 0};

				break;
			}
			i++;
		}

		tile.pos = oldTile.pos;
		tile.ppos = sf::Vector2f(tile.pos) * (float)32.0;
		tile.z = 0;

		tile.sprite.setTexture(texManager.getRef(rname));

		Animation staticAnim({0, 1, 2});

		AnimationHandler idleHandler;

		idleHandler.frameSize = sf::IntRect(0, 0, tile.psize.x, tile.psize.y);

		idleHandler.addAnim(staticAnim);
		idleHandler.changeColumn(0);
		idleHandler.set(0);

		tile.sprite.setTextureRect(idleHandler.bounds); // texture need to be updated

		tile.animHandlers["idle"] = idleHandler;

		tile.direction = North;
		tile.state = "idle";

		tile.centerRect = this->getCenterRect(name);

		registry.remove<Tile>(entity);
		registry.assign<Tile>(entity, tile);
		return entity;
	}

	EntityID createMapEffect(entt::Registry<EntityID> &registry, std::string name)
	{
		EntityID entity = registry.create();

#ifdef FACTORY_DEBUG
		std::cout << "EntityFactory: create map effect " << entity << " " << name << std::endl;
#endif

		Tile tile;
		tileParser.parse(tile, this->getXmlComponent(name, "tile"));
		tile.pos = sf::Vector2i(0, 0);
		tile.ppos = sf::Vector2f(tile.pos) * (float)32.0;
		tile.z = 1;

		tile.sprite.setTexture(texManager.getRef(name));

		tile.direction = North;
		tile.state = "fx";

		AnimationHandler &fxAnim = tile.animHandlers["fx"];

		fxAnim.changeColumn(0);
		fxAnim.set(0);
		tile.sprite.setTextureRect(fxAnim.bounds); // texture need to be updated

		MapEffect effect;
		effect.show = false;
		effect.speed = 0.0;
		effect.sound = name;

		registry.assign<Tile>(entity, tile);
		registry.assign<MapEffect>(entity, effect);

		return entity;
	}

// Player
	EntityID createPlayer(entt::Registry<EntityID> &registry, std::string team, bool ai) {
		EntityID entity = registry.create();

#ifdef FACTORY_DEBUG
		std::cout << "EntityFactory: create player " << entity << " " << team << std::endl;
#endif

		Player player;
		player.team = team;
		player.ai = ai;
		player.resources = 0;
		player.butchery = 0.0;
		player.enemyFound = false;
		player.rootConstruction = 0;

		player.stats["kills"] = 0;
		player.stats["combo"] = 0;
		player.stats["killer"] = 0;
		player.stats["megakill"] = 0;
		player.stats["barbarian"] = 0;
		player.stats["butchery"] = 0;

		if (team == "rebel")
			player.resourceType = ResourceType::Nature;
		else if (team == "neonaz")
			player.resourceType = ResourceType::Pollution;

		registry.assign<Player>(entity, player);
		return entity;
	}

	void loadManifest(std::string filename) {
		std::cout << "EntityFactory: load manifest " << filename << std::endl;

		tinyxml2::XMLDocument doc;
		doc.LoadFile(filename.c_str());

		for (tinyxml2::XMLElement *childEl : doc.RootElement()) {
			std::string name = childEl->Name();
			std::string sfilename = childEl->Attribute("path");

			tinyxml2::XMLDocument *sdoc = new tinyxml2::XMLDocument();
			sdoc->LoadFile(sfilename.c_str());

			this->loadedXmlDocs[texLoader.getName(sdoc->RootElement())] = sdoc;

			texLoader.parse(sdoc->RootElement());
			sndLoader.parse(sdoc->RootElement());
		}
		
		std::cout << "EntityFactory: manifest loaded " << filename << std::endl;
	}

	void load() {
		if (!this->loaded) {
			this->loadManifest("defs/new/manifest.xml");

			this->loadTerrains();

			this->loadResources("defs/res/nature.xml");
			this->loadResources("defs/res/pollution.xml");

			this->loadPlayerColors("defs/unit_color.xml");

			this->loadMisc();
			this->loadTechTrees();
			this->loaded = true;
		}
	}

	EntityFactory() {
		texLoader.setManager(&texManager);
		sndLoader.setManager(&sndManager);
		this->loaded = false;
	}

};