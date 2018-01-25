#pragma once

#include "Config.hpp"

#include "Components.hpp"
#include "TextureManager.hpp"
#include "SoundBufferManager.hpp"

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

	std::vector<std::string> unitFiles;
	std::vector<std::string> buildingFiles;

	std::map<std::string, TechNode> techTrees;

	std::map<std::string, int> resourcesCount;

	std::map<std::string, sf::IntRect> centerRects;
	std::map<int, std::vector<sf::Color> > playerColors;

public:
	TextureManager texManager;
	SoundBufferManager sndManager;

	sf::Texture &getTex(std::string name) {
		return texManager.getRef(name);
	}

	sf::SoundBuffer &getSndBuf(std::string name) {
		return sndManager.getRef(name);
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
		texManager.loadTexture(name, img, sf::IntRect{0, 0, (int)img.getSize().x, (int)img.getSize().y});

	}

	void loadInitial() {
		this->loadTextureWithWhiteMask("intro_background", "medias/interface/bgs/toile.png");

	}

	void loadMisc() {

		this->loadTextureWithWhiteMask("interface_rebel", "medias/interface/bgs/interface_rebel_800x600.png");
		this->loadTextureWithWhiteMask("interface_neonaz", "medias/interface/bgs/interface_neonaz_800x600.png");

		this->loadTextureWithWhiteMask("indice_bg_rebel", "medias/interface/bgs/indice_bio-bg.png");
		this->loadTextureWithWhiteMask("indice_bg_neonaz", "medias/interface/bgs/indice_pol-bg.png");

		this->loadTextureWithWhiteMask("indice_rebel", "medias/interface/bgs/indice_bio.png");
		this->loadTextureWithWhiteMask("indice_neonaz", "medias/interface/bgs/indice_pol.png");

		this->loadButton("rebel_move", "medias/interface/buttons/rebel_move_button.png");
		this->loadButton("rebel_attack", "medias/interface/buttons/rebel_attack_button.png");
		this->loadButton("rebel_cancel", "medias/interface/buttons/annuler_rebelle.png");

		this->loadButton("neonaz_move", "medias/interface/buttons/naz_move_button.png");
		this->loadButton("neonaz_attack", "medias/interface/buttons/naz_attack_button.png");
		this->loadButton("neonaz_cancel", "medias/interface/buttons/annuler_naz.png");

		this->loadBuildButton("nature_icon", "medias/resources/nature-icon.png");
		this->loadBuildButton("pollution_icon", "medias/resources/pollution-icon.png");

		this->loadTextureWithWhiteMask("button2", "medias/interface/buttons/button2.png");

		this->loadButton("menu_button", "medias/interface/buttons/menu_button.png");

		this->loadTextureWithWhiteMask("shadow", "medias/misc/shadow.png");
		this->loadTextureWithWhiteMask("selected", "medias/tiles/cadre_unit.png");
		this->loadTextureWithWhiteMask("forbid", "medias/misc/forbide.png");

		this->loadTextureWithWhiteMask("ruin", "medias/misc/ruine.png");

		this->loadTextureWithWhiteMask("explosion", "medias/misc/explosion.png");
		sndManager.loadSoundBuffer("explosion", "medias/misc/explosion.wav");
		this->loadTextureWithWhiteMask("blood", "medias/misc/blood.png");

		sndManager.loadSoundBuffer("combo", "medias/misc/combo.wav");
		sndManager.loadSoundBuffer("killer", "medias/misc/killer.wav");
		sndManager.loadSoundBuffer("megakill", "medias/misc/megakill.wav");
		sndManager.loadSoundBuffer("barbarian", "medias/misc/barbarian.wav");
		sndManager.loadSoundBuffer("butchery", "medias/misc/butchery.wav");
	}

	void autoTransition(sf::Image &img) {
		// TODO: use an XML to let modifiying PNG for better transitions
		for (int col = 0; col < 5; col++) {
			img.copy(img, col*32, 6*32, sf::IntRect(col*32, 2*32, 32, 32), true);
			img.copy(img, col*32, 6*32, sf::IntRect(col*32, 4*32, 32, 32), true);

			img.copy(img, col*32, 7*32, sf::IntRect(col*32, 1*32, 32, 32), true);
			img.copy(img, col*32, 7*32, sf::IntRect(col*32, 2*32, 32, 32), true);
			img.copy(img, col*32, 7*32, sf::IntRect(col*32, 4*32, 32, 32), true);

			img.copy(img, col*32, 9*32, sf::IntRect(col*32, 1*32, 32, 32), true);
			img.copy(img, col*32, 9*32, sf::IntRect(col*32, 8*32, 32, 32), true);

			img.copy(img, col*32, 11*32, sf::IntRect(col*32, 1*32, 32, 32), true);
			img.copy(img, col*32, 11*32, sf::IntRect(col*32, 2*32, 32, 32), true);
			img.copy(img, col*32, 11*32, sf::IntRect(col*32, 8*32, 32, 32), true);

			img.copy(img, col*32, 13*32, sf::IntRect(col*32, 1*32, 32, 32), true);
			img.copy(img, col*32, 13*32, sf::IntRect(col*32, 4*32, 32, 32), true);
			img.copy(img, col*32, 13*32, sf::IntRect(col*32, 8*32, 32, 32), true);

			img.copy(img, col*32, 14*32, sf::IntRect(col*32, 2*32, 32, 32), true);
			img.copy(img, col*32, 14*32, sf::IntRect(col*32, 4*32, 32, 32), true);
			img.copy(img, col*32, 14*32, sf::IntRect(col*32, 8*32, 32, 32), true);

			img.copy(img, col*32, 15*32, sf::IntRect(col*32, 1*32, 32, 32), true);
			img.copy(img, col*32, 15*32, sf::IntRect(col*32, 2*32, 32, 32), true);
			img.copy(img, col*32, 15*32, sf::IntRect(col*32, 4*32, 32, 32), true);
			img.copy(img, col*32, 15*32, sf::IntRect(col*32, 8*32, 32, 32), true);

			img.copy(img, col*32, (16+3)*32, sf::IntRect(col*32, (16+1)*32, 32, 32), true);
			img.copy(img, col*32, (16+3)*32, sf::IntRect(col*32, (16+2)*32, 32, 32), true);

			img.copy(img, col*32, (16+5)*32, sf::IntRect(col*32, (16+1)*32, 32, 32), true);
			img.copy(img, col*32, (16+5)*32, sf::IntRect(col*32, (16+4)*32, 32, 32), true);

			img.copy(img, col*32, (16+10)*32, sf::IntRect(col*32, (16+2)*32, 32, 32), true);
			img.copy(img, col*32, (16+10)*32, sf::IntRect(col*32, (16+8)*32, 32, 32), true);

			img.copy(img, col*32, (16+12)*32, sf::IntRect(col*32, (16+4)*32, 32, 32), true);
			img.copy(img, col*32, (16+12)*32, sf::IntRect(col*32, (16+8)*32, 32, 32), true);

			img.copy(img, col*32, (16+13)*32, sf::IntRect(col*32, (16+1)*32, 32, 32), true);
			img.copy(img, col*32, (16+13)*32, sf::IntRect(col*32, (16+4)*32, 32, 32), true);
			img.copy(img, col*32, (16+13)*32, sf::IntRect(col*32, (16+8)*32, 32, 32), true);

			img.copy(img, col*32, (16+14)*32, sf::IntRect(col*32, (16+2)*32, 32, 32), true);
			img.copy(img, col*32, (16+14)*32, sf::IntRect(col*32, (16+4)*32, 32, 32), true);
			img.copy(img, col*32, (16+14)*32, sf::IntRect(col*32, (16+8)*32, 32, 32), true);

			img.copy(img, col*32, (16+15)*32, sf::IntRect(col*32, (16+1)*32, 32, 32), true);
			img.copy(img, col*32, (16+15)*32, sf::IntRect(col*32, (16+2)*32, 32, 32), true);
			img.copy(img, col*32, (16+15)*32, sf::IntRect(col*32, (16+4)*32, 32, 32), true);
			img.copy(img, col*32, (16+15)*32, sf::IntRect(col*32, (16+8)*32, 32, 32), true);
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


//		sf::Image fogTransitions;
		texManager.loadTexture("fog_transition", "medias/new/fog.png");
//		fogTransitions.loadFromFile("medias/new/fow.png");
//		texManager.loadTexture("fog_transition", fogTransitions, sf::IntRect{0, 0, 32, 512});
//		texManager.loadTexture("fog_transition2", fogTransitions, sf::IntRect{32, 0, 32, 512});


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
		return this->centerRects[name];
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
#ifdef FACTORY_DEBUG
					std::cout << "EntityFactory: " << name << " special pix " << x << "x" << y << std::endl;
#endif
					if (found == 0) {
						pix1 = sf::Vector2i(x, y);
					} else if(found == 1) {
						pix2 = sf::Vector2i(x, y);
						sf::IntRect centerRect(pix1, pix2 - pix1);
#ifdef FACTORY_DEBUG
						std::cout << "EntityFactory: " << name << " center rect " << centerRect.left << "x" << centerRect.top << ":" << centerRect.width << "x" << centerRect.height << std::endl;
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

	// init unit texture with mirroring
	void initDirTexture(std::string name, std::string imgPath) {
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
#ifdef FACTORY_DEBUG
		std::cout << "EntityFactory: init dir texture " << name << " " << columnWidth << "x" << height << std::endl;
#endif
		texManager.loadTexture(name, outImage, sf::IntRect {0, 0, columnWidth * 8, height});
	}

	void loadUnits() {
		for (std::string &fn : this->unitFiles) {
			tinyxml2::XMLDocument *doc = new tinyxml2::XMLDocument();
			doc->LoadFile(fn.c_str());
			std::string name = doc->RootElement()->Attribute("name");
			this->docs[name] = doc;

			tinyxml2::XMLElement *root = doc->RootElement();
			std::string imgPath = root->FirstChildElement("file")->Attribute("path");
			this->initDirTexture(name, imgPath);

			this->loadButton(name + "_icon", root->FirstChildElement("icon")->Attribute("path"));

			texManager.loadTexture(name + "_face", root->FirstChildElement("face")->Attribute("path"));
			tinyxml2::XMLElement *speEl = root->FirstChildElement("spe");
			if (speEl)
				this->loadTextureWithWhiteMask(name + "_spe", speEl->Attribute("path"));

			tinyxml2::XMLElement * selSnds = root->FirstChildElement("select_sounds");
			if (selSnds) {
				int i = 0;
				for (tinyxml2::XMLElement *selSnd : selSnds) {
					sndManager.loadSoundBuffer(name + "_select_" + std::to_string(i), selSnd->Attribute("path"));
					i++;
				}
			}

			tinyxml2::XMLElement * statesEl = root->FirstChildElement("states");

			for (tinyxml2::XMLElement *stateEl : statesEl) {
				std::string stName = stateEl->Attribute("name");
				tinyxml2::XMLElement *sndsEl = stateEl->FirstChildElement("sounds");
				if (sndsEl) {
					int i = 0;
					for (tinyxml2::XMLElement *sndEl : sndsEl) {
						sndManager.loadSoundBuffer(name + "_" + stName + "_" + std::to_string(i), sndEl->Attribute("path"));
						i++;
					}
				}
			}

			tinyxml2::XMLElement * projEl = root->FirstChildElement("projectile");
			if (projEl) {
				std::string prName = projEl->Attribute("name");
				tinyxml2::XMLElement *fileEl = projEl->FirstChildElement("file");

				if (fileEl)
					this->initDirTexture(prName, fileEl->Attribute("path"));

				tinyxml2::XMLElement *sndEl = projEl->FirstChildElement("sounds")->FirstChildElement();
				if (sndEl) {
					sndManager.loadSoundBuffer(prName, sndEl->Attribute("path"));
				}
			}
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

	sf::Color getPlayerColor(sf::Color key, int idx) {
		return playerColors[key.r << 16 + key.g << 8 + key.b][idx];
	}

	Animation parseAnim(tinyxml2::XMLElement *stateEl, int pixFrame) {
		int refresh = stateEl->FirstChildElement("refresh")->IntAttribute("value");
		tinyxml2::XMLElement * framesEl = stateEl->FirstChildElement("frames");

		std::vector<int> frames;
		for (tinyxml2::XMLElement *frameEl : framesEl) {
			int frame = frameEl->IntAttribute("n");
//					std::cout << "ADD FRAME " << frame << std::endl;
			if (pixFrame == -1 || frame < pixFrame)
				frames.push_back(frame);
#ifdef BUG_DEBUG
			else
				std::cout << "BUG: invalid frame " << frame << " >= " << pixFrame << std::endl;
#endif
		}

		Animation anim(frames, 0.1f * refresh);

		return anim;
	}

	void parseTileFromXml(std::string name, Tile &tile, int directions) {
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
			std::string stateNm = stateEl->Attribute("name");
			int refresh = stateEl->FirstChildElement("refresh")->IntAttribute("value");
//			std::cout << "STATE " << stateNm << std::endl;

			AnimationHandler animHandler;

			animHandler.frameSize = sf::IntRect(0, 0, tile.psize.x, tile.psize.y);

			int pixFrame = texManager.getRef(name).getSize().y / tile.psize.y;

			for (int i = 0; i < directions; i++) {
				Animation anim = this->parseAnim(stateEl, pixFrame);
				/*
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

								Animation anim(frames, 0.1f * refresh);
				*/
				if (stateNm == "die") {
					anim.repeat = false;
					anim.duration = (0.1f * refresh) / 4.0;
				}

				animHandler.addAnim(anim);
			}
			animHandler.update(0.0f);

			tile.animHandlers[stateNm] = animHandler;

		}

	}

	void parseGameObjectFromXml(std::string name, GameObject &obj)
	{
		tinyxml2::XMLDocument *doc = this->docs[name];
		tinyxml2::XMLElement *root = doc->RootElement();

		obj.view = root->FirstChildElement("view")->IntAttribute("dist");
		obj.life = (float)root->FirstChildElement("life")->IntAttribute("value");
		obj.maxLife = obj.life;
		obj.name = root->Attribute("name");
		obj.team = root->Attribute("team");
	}

	void parseUnitFromXml(std::string name, Unit &unit) {
		tinyxml2::XMLDocument *doc = this->docs[name];
		tinyxml2::XMLElement *root = doc->RootElement();

		unit.speed = root->FirstChildElement("speed")->IntAttribute("value");

		unit.attack1 = Attack{(unsigned int)root->FirstChildElement("attack1")->IntAttribute("power"), 0};
		unit.attack2 = Attack{(unsigned int)root->FirstChildElement("attack2")->IntAttribute("power"), (unsigned int)root->FirstChildElement("attack2")->IntAttribute("dist")};


		tinyxml2::XMLElement * selSnds = root->FirstChildElement("select_sounds");
		if (selSnds) {
			int i = 0;
			for (tinyxml2::XMLElement *selSnd : selSnds) {
				i++;
			}
			unit.soundActions["select"] = i;
		} else {
			unit.soundActions["select"] = 0;

		}


		tinyxml2::XMLElement * statesEl = root->FirstChildElement("states");

		for (tinyxml2::XMLElement *stateEl : statesEl) {
			std::string stName = stateEl->Attribute("name");
			tinyxml2::XMLElement *sndsEl = stateEl->FirstChildElement("sounds");
			if (sndsEl) {
				int i = 0;
				for (tinyxml2::XMLElement *sndEl : sndsEl) {
					i++;
				}
				unit.soundActions[stName] = i;

			} else {
				unit.soundActions[stName] = 0;
			}
		}

		tinyxml2::XMLElement * projEl = root->FirstChildElement("projectile");
		if (projEl) {
			tinyxml2::XMLElement *fileEl = projEl->FirstChildElement("file");


			std::string prName = projEl->Attribute("name");
			unit.attackSound = prName;
//			unit.attackSound.setBuffer(this->sndManager.getRef(prName));
		}

	}

	void parseBuildingFromXml(std::string name, Building &building)
	{
		tinyxml2::XMLDocument *doc = this->docs[name];
		tinyxml2::XMLElement *root = doc->RootElement();

//		building.buildTime = 10;
		building.buildTime = (float)root->FirstChildElement("build_time")->IntAttribute("value");
		building.maxBuildTime = building.buildTime;
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

		idleHandler.changeAnim(0);
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
		this->parseTileFromXml(name, tile, 8);

		tile.pos = sf::Vector2i(x, y);
		tile.ppos = sf::Vector2f(tile.pos) * (float)32.0;
		tile.z = 0;

		tile.sprite.setTexture(texManager.getRef(name));

		tile.direction = South;
		tile.state = "idle";
		tile.sprite.setTextureRect(tile.animHandlers["idle"].bounds); // texture need to be updated

		tile.centerRect = this->centerRects[name];

		GameObject obj;
		this->parseGameObjectFromXml(name, obj);
		this->addProjectileFromXml(registry, name, obj );
		obj.player = player;
		obj.mapped = true;
		obj.destroy = false;

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

// Building
	EntityID startBuilding(entt::Registry<EntityID> &registry, std::string name, EntityID constructedBy) {
		EntityID entity = registry.create();
#ifdef FACTORY_DEBUG
		std::cout << "EntityFactory: start building " << entity << " " << name << " constructed by " << constructedBy << std::endl;
#endif
		Building building;
		this->parseBuildingFromXml(name, building);
		building.construction = 0;
		building.constructedBy = constructedBy;

		GameObject obj;
		this->parseGameObjectFromXml(name, obj);
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
		obj.player = player;
		obj.mapped = built;
		obj.effects["explosion"] = this->createExplosionEffect(registry);

		Tile tile;
		this->parseTileFromXml(obj.name, tile, 8);

		tile.pos = sf::Vector2i(x, y);
		tile.ppos = sf::Vector2f(tile.pos) * (float)32.0;
		tile.z = 0;

		tile.sprite.setTexture(texManager.getRef(obj.name));

		tile.direction = North;
		tile.state = "idle";
		tile.sprite.setTextureRect(tile.animHandlers["idle"].bounds); // texture need to be updated

		tile.centerRect = this->centerRects[obj.name];

//		obj.life = obj.life * (tile.size.x * tile.size.y) / 2;
//		obj.maxLife = obj.life;

		registry.assign<Tile>(entity, tile);

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
//				tile.size = sf::Vector2i{1, 1};
				tile.psize = sf::Vector2f{(float)psizeEl->IntAttribute("w"), (float)psizeEl->IntAttribute("h")};
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

//		std::cout << "growedResource: " << rnd << " " << rname << " " << tile.size.x << "x" << tile.size.y << std::endl;
		tile.pos = oldTile.pos;
		tile.ppos = sf::Vector2f(tile.pos) * (float)32.0;
		tile.z = 0;

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

	EntityID createMapEffect(entt::Registry<EntityID> &registry, std::string name,
	                         int w, int h, sf::IntRect centerRect, std::initializer_list<int> frames, float duration, int directions, int z) {
		Animation anim(frames, duration);
		return this->createMapEffect(registry, name, w, h, centerRect, anim, directions, z);
	}

	EntityID createMapEffect(entt::Registry<EntityID> &registry, std::string name,
	                         int w, int h, sf::IntRect centerRect, Animation anim, int directions, int z) {
		EntityID entity = registry.create();

#ifdef FACTORY_DEBUG
		std::cout << "EntityFactory: create map effect " << entity << " " << name << " " << w << "x" << h << " " << directions << std::endl;
#endif
		Tile tile;

		tile.size = sf::Vector2i(1, 1);
		tile.psize = sf::Vector2f(w, h);
		tile.pos = sf::Vector2i(0, 0);
		tile.ppos = sf::Vector2f(tile.pos) * (float)32.0;
		tile.z = z;

		tile.sprite.setTexture(texManager.getRef(name));

		tile.direction = North;
		tile.state = "fx";

		AnimationHandler fxAnim;
		fxAnim.frameSize = sf::IntRect(0, 0, tile.psize.x, tile.psize.y);

		for (int i = 0; i < directions; i++) {
			fxAnim.addAnim(anim);
		}
		fxAnim.changeAnim(0);
		fxAnim.set(0);
		tile.sprite.setTextureRect(fxAnim.bounds); // texture need to be updated

		tile.animHandlers["fx"] = fxAnim;

		MapEffect effect;
		effect.show = false;
		effect.speed = 0.0;
		effect.sound = name;
//		effect.sound.setBuffer(sndManager.getRef(name));

		tile.centerRect = centerRect;
		registry.assign<Tile>(entity, tile);
		registry.assign<MapEffect>(entity, effect);

		return entity;
	}

	EntityID createExplosionEffect(entt::Registry<EntityID> &registry) {
		return this->createMapEffect(registry, "explosion", 288, 280, sf::IntRect(128, 238, 32, 32), {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}, 0.1, 1, 1);
	}

	void addProjectileFromXml(entt::Registry<EntityID> &registry, std::string name, GameObject &obj) {
		tinyxml2::XMLDocument *doc = this->docs[name];
		tinyxml2::XMLElement *root = doc->RootElement();

		// projectile effect
		tinyxml2::XMLElement * projEl = root->FirstChildElement("projectile");
		if (projEl) {
			tinyxml2::XMLElement *fileEl = projEl->FirstChildElement("file");
			if (fileEl) {
				std::string prName = projEl->Attribute("name");
				tinyxml2::XMLElement * psizeEl = projEl->FirstChildElement("pixel_size");
				Animation anim = this->parseAnim(projEl, -1);
				EntityID projEnt = this->createMapEffect(registry, prName, psizeEl->IntAttribute("w"), psizeEl->IntAttribute("h"), sf::IntRect(0, 0, 32, 32), anim, 8, 1);

#ifdef FACTORY_DEBUG
				std::cout << "EntityFactory: add projectile " << projEnt << " to " << name << std::endl;
#endif
				obj.effects["projectile"] = projEnt;
			}
		}
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
		tinyxml2::XMLDocument doc;
		doc.LoadFile(filename.c_str());

		for (tinyxml2::XMLElement *childEl : doc.RootElement()) {
			std::string name = childEl->Name();

			if (name == "unit") {
				unitFiles.push_back(childEl->Attribute("path"));
			} else if (name == "building") {
				buildingFiles.push_back(childEl->Attribute("path"));
			}
		}
	}

	void load() {
		if (!this->loaded) {
			this->loadManifest("defs/manifest.xml");

			this->loadTerrains();
			this->loadUnits();
			this->loadBuildings();

			this->loadResources("defs/res/nature.xml");
			this->loadResources("defs/res/pollution.xml");

			this->loadPlayerColors("defs/unit_color.xml");

			this->loadMisc();
			this->loadTechTrees();
			this->loaded = true;
		}
	}

	EntityFactory() {
		this->loaded = false;
	}

};