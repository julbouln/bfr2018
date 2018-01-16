#pragma once

#include "Components.hpp"
#include "TextureManager.hpp"

#include "tinyxml2.h"
#include "tixml2ex.h"


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

		std::cout << "tech tree parse " << el->Name() << " " << type << " " << (int)comp << std::endl;

		for (tinyxml2::XMLElement *childEl : el) {
			TechNode childNode;
			childNode.parentType = this->type;
			childNode.parse(childEl);
			children.push_back(childNode);
		}
	}
};

class EntityFactory {

	std::map<std::string, tinyxml2::XMLDocument *> docs;

	std::vector<std::string> unitFiles;
	std::vector<std::string> buildingFiles;

	std::map<std::string, TechNode> techTrees;

	std::map<std::string, int> resourcesCount;

	std::map<std::string, sf::IntRect> centerRects;

public:
	TextureManager texManager;
	sf::Texture &getTex(std::string name) {
		return texManager.getRef(name);
	}

	void loadButton(std::string name, std::string filename) {
		sf::Image but;
		but.loadFromFile(filename);
		texManager.loadTexture(name, but, sf::IntRect{0, 0, (int)but.getSize().x, (int)but.getSize().y / 2});
		texManager.loadTexture(name + "_down", but, sf::IntRect{0, (int)(but.getSize().y / 2), (int)but.getSize().x, (int)(but.getSize().y) / 2});
	}

	void loadBuildButton(std::string name, std::string filename) {
		sf::Image but;
		but.loadFromFile(filename);
		int height = (int)but.getSize().y / 5;
		texManager.loadTexture(name, but, sf::IntRect{0, 0, (int)but.getSize().x, height});
		texManager.loadTexture(name + "_down", but, sf::IntRect{0, height, (int)but.getSize().x, height});
		texManager.loadTexture(name + "_building", but, sf::IntRect{0, height * 2, (int)but.getSize().x, height});
		texManager.loadTexture(name + "_built", but, sf::IntRect{0, height * 3, (int)but.getSize().x, height});
		texManager.loadTexture(name + "_built_down", but, sf::IntRect{0, height * 4, (int)but.getSize().x, height});
	}

	void loadTextureWithWhiteMask(std::string name, std::string filename) {
		sf::Image img;
		img.loadFromFile(filename);
		this->getSpecialPix(name, img, img.getSize().x, img.getSize().y);
		img.createMaskFromColor(sf::Color::White);
		texManager.loadTexture(name, img, sf::IntRect{0, 0, img.getSize().x, img.getSize().y});

	}

	void loadMisc() {
		this->loadTextureWithWhiteMask("interface_rebel", "medias/interface/bgs/interface_rebel_800x600.png");
		this->loadTextureWithWhiteMask("interface_neonaz", "medias/interface/bgs/interface_neonaz_800x600.png");

		this->loadButton("rebel_move", "medias/interface/buttons/rebel_move_button.png");
		this->loadButton("rebel_attack", "medias/interface/buttons/rebel_attack_button.png");

		this->loadButton("neonaz_move", "medias/interface/buttons/naz_move_button.png");
		this->loadButton("neonaz_attack", "medias/interface/buttons/naz_attack_button.png");

		this->loadBuildButton("nature_icon", "medias/resources/nature-icon.png");
		this->loadBuildButton("pollution_icon", "medias/resources/pollution-icon.png");
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
		transitions.loadFromFile("medias/tiles/bordures.png");
		transitions.createMaskFromColor(sf::Color::White);
		texManager.loadTexture("dirt_transition", transitions, sf::IntRect{96, 0, 32, 640});

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

	TechNode *recGetTechNode(TechNode *node, std::string type) {
		if (node->type == type)
			return node;
		else
			for (TechNode &child : node->children) {
				TechNode *cnode = recGetTechNode(&child, type);
				if (cnode)
					return cnode;
			}
		return nullptr;
	}

	TechNode *getTechNode(std::string team, std::string type) {
		return this->recGetTechNode(&this->techTrees[team], type);
	}

	TechNode *getTechRoot(std::string team) {
		return &this->techTrees[team];
	}

	void getSpecialPix(std::string name, sf::Image &image, int width, int height) {
		sf::Vector2i pix1;
		sf::Vector2i pix2;
		int found = 0;
		for (int x = 0; x < width; x++) {
			for (int y = 0; y < height; y++) {
				if(found > 1)
					return;
				if (image.getPixel(x, y) == sf::Color(255, 36, 196)) {
					std::cout << name << " special pix "<<x<<"x"<<y<<std::endl;
					if(found==0) {
						pix1= sf::Vector2i(x,y);
					} else {
						pix2 = sf::Vector2i(x,y);
						sf::IntRect centerRect(pix1,pix2-pix1);
						std::cout << name << " center rect " << centerRect.left << "x" << centerRect.top << ":" << centerRect.width << "x" << centerRect.height << std::endl;
						centerRects[name]=centerRect;
					}

					found++;
				}
			}
		}

		if(found) {

		}

	}

	// init unit texture with mirroring
	void initUnitTexture(std::string name, std::string imgPath) {
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
		texManager.loadTexture(name, outImage, sf::IntRect{0, 0, columnWidth * 8, height});
	}

	void loadUnits() {
		for (std::string &fn : this->unitFiles) {
			tinyxml2::XMLDocument *doc = new tinyxml2::XMLDocument();
			doc->LoadFile(fn.c_str());
			std::string name = doc->RootElement()->Attribute("name");
			this->docs[name] = doc;

			std::string imgPath = doc->RootElement()->FirstChildElement("file")->Attribute("path");
			this->initUnitTexture(name, imgPath);

			this->loadButton(name + "_icon", doc->RootElement()->FirstChildElement("icon")->Attribute("path"));

			texManager.loadTexture(name + "_face", doc->RootElement()->FirstChildElement("face")->Attribute("path"));
			tinyxml2::XMLElement *speEl = doc->RootElement()->FirstChildElement("spe");
			if (speEl)
				texManager.loadTexture(name + "_spe", speEl->Attribute("path"));
		}

	}

	void loadBuildings() {
		for (std::string &fn : this->buildingFiles) {
			tinyxml2::XMLDocument *doc = new tinyxml2::XMLDocument();
			doc->LoadFile(fn.c_str());
			std::string name = doc->RootElement()->Attribute("name");
			this->docs[name] = doc;

			std::string imgPath = doc->RootElement()->FirstChildElement("file")->Attribute("path");
			this->loadTextureWithWhiteMask(name, imgPath);

			this->loadBuildButton(name + "_icon", doc->RootElement()->FirstChildElement("icon")->Attribute("path"));
		}
	}

	void loadResources(std::string filename) {
		tinyxml2::XMLDocument *doc = new tinyxml2::XMLDocument();
		doc->LoadFile(filename.c_str());
		std::string name = doc->RootElement()->Attribute("name");
		this->docs[name] = doc;

		this->loadTextureWithWhiteMask(name, "medias/resources/" + name + ".png");

		int i = 1;
		for (tinyxml2::XMLElement *el : doc->RootElement()) {

			std::string imgfile = el->FirstChildElement("file")->Attribute("path");
//			std::cout << "RESOURCE: " << imgfile << std::endl;
			this->loadTextureWithWhiteMask(name + std::to_string(i), imgfile);
			i++;
		}
		resourcesCount[name] = i - 1;
	}

	void parseTileFromXml(std::string name,  Tile &tile, int directions) {
		tinyxml2::XMLDocument *doc = this->docs[name];
		tinyxml2::XMLElement *root = doc->RootElement();
		tinyxml2::XMLElement * sizeEl = root->FirstChildElement("case_size");
		tinyxml2::XMLElement * psizeEl = root->FirstChildElement("pixel_size");
		tinyxml2::XMLElement * offsetEl = root->FirstChildElement("decal_value");
		tinyxml2::XMLElement * statesEl = root->FirstChildElement("states");

		tile.size = sf::Vector2i{sizeEl->IntAttribute("w"), sizeEl->IntAttribute("h")};
		tile.psize = sf::Vector2f{(float)psizeEl->IntAttribute("w"), (float)psizeEl->IntAttribute("h")};

		tile.offset = sf::Vector2i{offsetEl->IntAttribute("x"), offsetEl->IntAttribute("y")};

		for (tinyxml2::XMLElement *stateEl : statesEl) {
			std::string state = stateEl->Attribute("name");
//			std::cout << "STATE " << state << std::endl;

			AnimationHandler animHandler;

			animHandler.frameSize = sf::IntRect(0, 0, tile.psize.x, tile.psize.y);

			int pixFrame = texManager.getRef(name).getSize().y / tile.psize.y;

			for (int i = 0; i < directions; i++) {
				tinyxml2::XMLElement * framesEl = stateEl->FirstChildElement("frames");

				std::vector<int> frames;
				for (tinyxml2::XMLElement *frameEl : framesEl) {
					int frame = frameEl->IntAttribute("n");
//					std::cout << "ADD FRAME " << frame << std::endl;
					if (frame < pixFrame)
						frames.push_back(frame);
					else
						std::cout << "BUG: invalid frame " << frame << " >= " << pixFrame << std::endl;
				}

				Animation anim(frames, 0.5f);

				animHandler.addAnim(anim);
			}
			animHandler.update(0.0f);

			tile.animHandlers[state] = animHandler;

		}

	}

	void parseGameObjectFromXml(std::string name, GameObject &obj)
	{
		tinyxml2::XMLDocument *doc = this->docs[name];
		tinyxml2::XMLElement *root = doc->RootElement();

		obj.view = root->FirstChildElement("view")->IntAttribute("dist");
		obj.life = (float)root->FirstChildElement("life")->IntAttribute("value");
		obj.name = root->Attribute("name");
		obj.team = root->Attribute("team");
	}

	void parseUnitFromXml(std::string name, Unit &unit) {
		tinyxml2::XMLDocument *doc = this->docs[name];
		tinyxml2::XMLElement *root = doc->RootElement();

		unit.speed = root->FirstChildElement("speed")->IntAttribute("value");

		unit.attack1 = Attack{(unsigned int)root->FirstChildElement("attack1")->IntAttribute("power"), 0};
		unit.attack2 = Attack{(unsigned int)root->FirstChildElement("attack2")->IntAttribute("power"), (unsigned int)root->FirstChildElement("attack2")->IntAttribute("dist")};
	}

	void parseBuildingFromXml(std::string name, Building &building)
	{
		tinyxml2::XMLDocument *doc = this->docs[name];
		tinyxml2::XMLElement *root = doc->RootElement();

		building.buildTime = root->FirstChildElement("build_time")->IntAttribute("value");
	}

	EntityID createTerrain(entt::Registry<EntityID> &registry, std::string name, int variant) {
		EntityID entity = registry.create();
		Tile tile;
		tile.psize = sf::Vector2f{32, 32};
		tile.size = sf::Vector2i{1, 1};

		tile.pos = sf::Vector2i(0, 0);
		tile.ppos = sf::Vector2f(tile.pos) * (float)32.0;

		tile.sprite.setTexture(texManager.getRef(name));

		Animation staticAnim({0, 1, 2});

		AnimationHandler idleHandler;

		idleHandler.frameSize = sf::IntRect(0, 0, 32, 32);

		idleHandler.addAnim(staticAnim);

		idleHandler.changeAnim(0);
		idleHandler.set(variant);

		tile.sprite.setTextureRect(idleHandler.bounds); // texture need to be updated


		tile.animHandlers["idle"] = idleHandler;

		tile.direction = North;
		tile.state = "idle";

		registry.assign<Tile>(entity, tile);
		return entity;
	}

#define UNIT_FRAME_COUNT 10

	EntityID createUnit(entt::Registry<EntityID> &registry, EntityID player, std::string name, int x, int y) {
		EntityID entity = registry.create();
		Tile tile;
		this->parseTileFromXml(name, tile, 8);

		tile.pos = sf::Vector2i(x, y);
		tile.ppos = sf::Vector2f(tile.pos) * (float)32.0;

//		tile.sprite.setOrigin(sf::Vector2f(tile.psize.x / 2, tile.psize.y / 2));
//		tile.sprite.setOrigin(sf::Vector2f(16,16));
		tile.sprite.setTexture(texManager.getRef(name));

		tile.direction = South;
		tile.state = "idle";

		tile.centerRect = this->centerRects[name];

		GameObject obj;
		this->parseGameObjectFromXml(name, obj);
		obj.player = player;
		obj.mapped = true;

		Unit unit;
		this->parseUnitFromXml(name, unit);

		unit.nextpos = tile.pos;
		unit.destAttack = 0;
		unit.nopath = 0;
		unit.destpos = tile.pos;

		registry.assign<Tile>(entity, tile);
		registry.assign<GameObject>(entity, obj);
		registry.assign<Unit>(entity, unit);

		return entity;
	}

	EntityID createBuilding(entt::Registry<EntityID> &registry, EntityID player, std::string name, int x, int y, bool built) {
		EntityID entity = registry.create();
		Tile tile;
		this->parseTileFromXml(name, tile, 8);

		tile.pos = sf::Vector2i(x, y);
		tile.ppos = sf::Vector2f(tile.pos) * (float)32.0;


//		tile.sprite.setOrigin(sf::Vector2f(tile.psize.x / 2, tile.psize.y / 2));
		tile.sprite.setTexture(texManager.getRef(name));

//		Animation staticAnim({}, 1.0f);

//		AnimationHandler idleHandler;

//		idleHandler.frameSize = sf::IntRect(0, 0, tile.psize.x, tile.psize.y);

//		idleHandler.addAnim(staticAnim);
//		idleHandler.update(0.0f);

//		tile.animHandlers["idle"] = idleHandler;

		tile.direction = North;
		tile.state = "idle";

		tile.centerRect = this->centerRects[name];

		GameObject obj;
		this->parseGameObjectFromXml(name, obj);
		obj.player = player;
		obj.mapped = built;

		Building building;
		this->parseBuildingFromXml(name, building);

		registry.assign<Tile>(entity, tile);
		registry.assign<GameObject>(entity, obj);
		registry.assign<Building>(entity, building);

		return entity;
	}


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

		std::string name = this->resourceTypeName(type);
		Tile tile;
		tile.psize = sf::Vector2f{32, 32};
		tile.size = sf::Vector2i{1, 1};

		tile.pos = sf::Vector2i(x, y);
		tile.ppos = sf::Vector2f(tile.pos) * (float)32.0;

//		tile.sprite.setOrigin(sf::Vector2f(16, 16));
		tile.sprite.setTexture(texManager.getRef(name));

		Animation staticAnim({0});

		AnimationHandler idleHandler;

		idleHandler.frameSize = sf::IntRect(0, 0, 32, 32);

		idleHandler.addAnim(staticAnim);
		idleHandler.update(0.0f);

		tile.animHandlers["idle"] = idleHandler;

		tile.direction = North;
		tile.state = "idle";

		tile.centerRect = this->centerRects[name];

		Resource resource;
		resource.type = type;
		resource.level = 0;
		resource.grow = 0;

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
//				tile.size = sf::Vector2i{1, 1};
				tile.psize = sf::Vector2f{psizeEl->IntAttribute("w"), psizeEl->IntAttribute("h")};
				if (offsetEl) {
//					tile.offset = sf::Vector2i{offsetEl->IntAttribute("w"), offsetEl->IntAttribute("h")};
					tile.offset = sf::Vector2i{0, offsetEl->IntAttribute("h")};
				}
				else
					tile.offset = sf::Vector2i{0, 0};

//				tile.offset = sf::Vector2i{0, 0};
//				if (offsetEl)
//				tile.offset = sf::Vector2i{0, sizeEl->IntAttribute("h")-1};

				break;
			}
			i++;
		}

		std::cout << "growedResource: " << rnd << " " << rname << " " << tile.size.x << "x" << tile.size.y << std::endl;
		tile.pos = oldTile.pos;
		tile.ppos = sf::Vector2f(tile.pos) * (float)32.0;

		tile.sprite.setTexture(texManager.getRef(rname));

		Animation staticAnim({0, 1, 2});

		AnimationHandler idleHandler;

		idleHandler.frameSize = sf::IntRect(0, 0, tile.psize.x, tile.psize.y);

		idleHandler.addAnim(staticAnim);
		idleHandler.changeAnim(0);
		idleHandler.set(0);

		tile.sprite.setTextureRect(idleHandler.bounds); // texture need to be updated

		tile.animHandlers["idle"] = idleHandler;

		tile.direction = North;
		tile.state = "idle";

		tile.centerRect = this->centerRects[rname];

		registry.remove<Tile>(entity);
		registry.assign<Tile>(entity, tile);
		return entity;
	}

	EntityID createPlayer(entt::Registry<EntityID> &registry, std::string team, bool ai) {
		EntityID entity = registry.create();
		Player player;
		player.team = team;
		player.ai = ai;
		player.resources = 0;
		player.butchery = 0;

		if (team == "rebel")
			player.resourceType = ResourceType::Nature;
		else
			player.resourceType = ResourceType::Pollution;

		registry.assign<Player>(entity, player);
		return entity;
	}

	void load() {
		unitFiles.push_back("defs/uni/patrouilleur.xml");
		unitFiles.push_back("defs/uni/punkette.xml");
		unitFiles.push_back("defs/uni/guerrier_bud.xml");
		unitFiles.push_back("defs/uni/brad_lab.xml");
		unitFiles.push_back("defs/uni/guerrier_bud_powerhead.xml");
		unitFiles.push_back("defs/uni/lance_pepino.xml");
		unitFiles.push_back("defs/uni/zork.xml");
		unitFiles.push_back("defs/uni/super_guerrier.xml");
		unitFiles.push_back("defs/uni/abdel.xml");
		unitFiles.push_back("defs/uni/grosnaz.xml");
		unitFiles.push_back("defs/uni/bazooka.xml");
		unitFiles.push_back("defs/uni/lance_missille.xml");
		unitFiles.push_back("defs/uni/mitrailleur.xml");

		buildingFiles.push_back("defs/bui/artillerie.xml");
		buildingFiles.push_back("defs/bui/caserne.xml");
		buildingFiles.push_back("defs/bui/ferme.xml");
		buildingFiles.push_back("defs/bui/festival.xml");
		buildingFiles.push_back("defs/bui/gymnaz.xml");
		buildingFiles.push_back("defs/bui/labo.xml");
		buildingFiles.push_back("defs/bui/mirador.xml");
		buildingFiles.push_back("defs/bui/raffinerie_bud.xml");
		buildingFiles.push_back("defs/bui/squat.xml");
		buildingFiles.push_back("defs/bui/taverne.xml");
		buildingFiles.push_back("defs/bui/tourelle.xml");
		buildingFiles.push_back("defs/bui/usine_vehicule.xml");

		this->loadTerrains();
		this->loadUnits();
		this->loadBuildings();

		this->loadResources("defs/res/nature.xml");
		this->loadResources("defs/res/pollution.xml");

		this->loadMisc();
		this->loadTechTrees();
	}

	EntityFactory() {

	}

};