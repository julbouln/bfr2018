#include "EntityFactory.hpp"

#define UNIT_FRAME_COUNT 10

sf::Texture &EntityFactory::getTex(std::string name) {
	return texManager.getRef(name);
}

sf::SoundBuffer &EntityFactory::getSndBuf(std::string name) {
	return sndManager.getRef(name);
}

void EntityFactory::loadInitial() {
	fntManager.load("samos", "medias/fonts/samos.ttf");
	texLoader.loadTextureWithWhiteMask("intro_background", "medias/interface/bgs/toile.png");
	texLoader.loadTextureWithWhiteMask("cursors", "medias/extra/curseurs.png");
//	texLoader.loadTextureWithWhiteMask("testninepatch", "medias/interface/motifs/motif_bouton1.png");
}

void EntityFactory::loadMisc() {
#ifdef SHADER_ENABLE
	shrManager.load("color_swap", "defs/shaders/color_swap.frag");
	shrManager.load("pixelation", "defs/shaders/pixelation.frag");
	shrManager.load("outline", "defs/shaders/outline.frag");
//		shrManager.load("metaball", particles::metaballVertexShader, particles::metaballFragmentShader);
#endif

	texLoader.loadTextureWithWhiteMask("interface_rebel", "medias/interface/bgs/interface_rebel.png");
	texLoader.loadTextureWithWhiteMask("interface_neonaz", "medias/interface/bgs/interface_neonaz.png");

	texLoader.loadTextureWithWhiteMask("minimap_rebel", "medias/interface/bgs/minimap_rebel.png");
	texLoader.loadTextureWithWhiteMask("minimap_neonaz", "medias/interface/bgs/minimap_neonaz.png");

	texLoader.loadTextureWithWhiteMask("box_rebel", "medias/interface/bgs/boite_rebel.png");
	texLoader.loadTextureWithWhiteMask("box_neonaz", "medias/interface/bgs/boite_neonaz.png");

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

	texLoader.loadTextureWithWhiteMask("rebel_face", "medias/extra/rebel-face.png");
	texLoader.loadTextureWithWhiteMask("neonaz_face", "medias/extra/neonaz-face.png");

	texLoader.loadButton("move", "medias/interface/buttons/deplacement.png");
	texLoader.loadButton("attack", "medias/interface/buttons/attaque.png");

	texLoader.loadBuildButton("nature_icon", "medias/resources/nature-icon.png");
	texLoader.loadButton("pollution_icon", "medias/resources/pollution-icon.png");

	texLoader.loadTextureWithWhiteMask("button2", "medias/interface/buttons/button2.png");

	texLoader.loadButton("menu_button", "medias/interface/buttons/menu_button.png");

	texLoader.loadTextureWithWhiteMask("shadow", "medias/misc/shadow.png");
	texLoader.loadTextureWithWhiteMask("selected", "medias/tiles/cadre_unit.png");
	texLoader.loadTextureWithWhiteMask("forbid", "medias/misc/forbide.png");

	texLoader.loadTextureWithWhiteMask("ruin", "medias/misc/ruine.png");

	texLoader.loadTextureWithWhiteMask("blood", "medias/misc/blood.png");

	sndManager.load("combo", "medias/misc/combo.flac");
	sndManager.load("killer", "medias/misc/killer.flac");
	sndManager.load("megakill", "medias/misc/megakill.flac");
	sndManager.load("barbarian", "medias/misc/barbarian.flac");
	sndManager.load("butchery", "medias/misc/butchery.flac");
	sndManager.load("pause_on", "medias/misc/pause_on.flac");
	sndManager.load("pause_off", "medias/misc/pause_off.flac");

	texLoader.loadTextureWithWhiteMask("pollution_cost", "medias/extra/baril_ico.png");
	texLoader.loadTextureWithWhiteMask("nature_cost", "medias/extra/pepino_ico.png");
	texManager.load("time", "medias/extra/temps.png");

	texManager.load("arrow", "medias/new/fleche.png");


}

void EntityFactory::autoTransition(sf::Image &img) {
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

void EntityFactory::loadTerrains() {
	sf::Image terrains;
	terrains.loadFromFile("medias/tiles/terrains.png");
	texManager.load("sand", terrains, sf::IntRect{0, 0, 32, 96});
	texManager.load("water", terrains, sf::IntRect{32, 0, 32, 96});
	texManager.load("grass", terrains, sf::IntRect{64, 0, 32, 96});
	texManager.load("dirt", terrains, sf::IntRect{96, 0, 32, 96});
	texManager.load("concrete", terrains, sf::IntRect{128, 0, 32, 96});

	texManager.load("terrains", "medias/tiles/terrains.png");

	sf::Image transitions;
	transitions.loadFromFile("medias/new/transitions.png");
	transitions.createMaskFromColor(sf::Color::White);

	this->autoTransition(transitions);

	texManager.load("sand_transition", transitions, sf::IntRect{0, 0, 32, 1024});
	texManager.load("water_transition", transitions, sf::IntRect{32, 0, 32, 1024});
	texManager.load("dirt_transition", transitions, sf::IntRect{96, 0, 32, 1024});
	texManager.load("concrete_transition", transitions, sf::IntRect{128, 0, 32, 1024});

	texManager.load("terrains_transitions", transitions, sf::IntRect{0, 0, 160, 1024});

	texManager.load("fog_transition", "medias/new/fog.png");
	texManager.load("debug_transition", "medias/new/debug_transitions256.png");
}

void EntityFactory::loadDecorGenerator(std::string filename) {
	tinyxml2::XMLDocument doc;
	doc.LoadFile(filename.c_str());

	for (tinyxml2::XMLElement *el : doc.RootElement()) {
		decorGenerator[el->Attribute("group")] = el->IntAttribute("fact");
	}
}

TechNode EntityFactory::loadTechTree(std::string filename) {
	tinyxml2::XMLDocument doc;
	doc.LoadFile(filename.c_str());
	TechNode tech;
	tech.parse(doc.RootElement()->FirstChildElement());

	return tech;
}

void EntityFactory::loadTechTrees() {
	techTrees["rebel"] = this->loadTechTree("defs/tech/rebels.xml");
	techTrees["neonaz"] = this->loadTechTree("defs/tech/neonaz.xml");
}

TechNode *EntityFactory::recGetTechNodeByName(TechNode *node, std::string type) {
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

std::vector<TechNode *> EntityFactory::recGetTechNodes(TechNode *node, std::vector<TechNode *> currentNodes) {
	std::vector<TechNode *>nodes = currentNodes;
	for (TechNode &child : node->children) {
		nodes.push_back(&child);
		nodes = recGetTechNodes(&child, nodes);
	}
	return nodes;
}

std::vector<TechNode *> EntityFactory::getTechNodes(std::string team) {
	std::vector<TechNode *> nodes;
	nodes = this->recGetTechNodes(&techTrees[team], nodes);
	return nodes;
}

TechNode *EntityFactory::getTechNode(std::string team, std::string type) {
	return this->recGetTechNodeByName(&this->techTrees[team], type);
}

TechNode *EntityFactory::getTechRoot(std::string team) {
	return &this->techTrees[team];
}

sf::IntRect EntityFactory::getCenterRect(std::string name) {
	return texLoader.centerRects[name];
}

sf::Color EntityFactory::getPlayerColor(sf::Color key, int idx) {
	return playerColors[key.r << 16 + key.g << 8 + key.b][idx];
}

// XML loader

void EntityFactory::loadPlayerColors(std::string filename) {
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

tinyxml2::XMLElement *EntityFactory::getXmlComponent(std::string name, const char* component) {
	if (this->loadedXmlDocs.count(name) > 0)
		return this->loadedXmlDocs[name]->RootElement()->FirstChildElement(component);
	else
		return nullptr;
}

void EntityFactory::parseTileFromXml(std::string name, Tile &tile) {
	tileParser.parse(tile, this->getXmlComponent(name, "tile"));
	tile.sprite.setTexture(texManager.getRef(name));

	tile.sprite.setTextureRect(sf::IntRect(0, 0, tile.psize.x, tile.psize.y));

	tile.centerRect = this->getCenterRect(name);
}


void EntityFactory::parseBuildingFromXml(std::string name, Building &building) {
	buildingParser.parse(building, this->getXmlComponent(name, "building"));
}

void EntityFactory::parseUnitFromXml(std::string name, Unit &unit) {
	unitParser.parse(unit, this->getXmlComponent(name, "unit"));
}

void EntityFactory::parseGameObjectFromXml(std::string name, GameObject &obj) {
	gameObjectParser.parse(obj, this->getXmlComponent(name, "game_object"));
}

void EntityFactory::parseResourceFromXml(std::string name, Resource &resource) {
	resourceParser.parse(resource, this->getXmlComponent(name, "resource"));
}

void EntityFactory::parseDecorFromXml(std::string name, Decor &decor) {
	decorParser.parse(decor, this->getXmlComponent(name, "decor"));
}


float EntityFactory::buildTime(std::string type) {
	Building building;
	this->parseBuildingFromXml(type, building);
	return building.maxBuildTime;
}

float EntityFactory::trainCost(std::string type) {
	Unit unit;
	this->parseUnitFromXml(type, unit);
	return unit.cost;
}

// get object initial life from XML
float EntityFactory::objTypeLife(std::string type) {
	GameObject obj;
	this->parseGameObjectFromXml(type, obj);
	return obj.maxLife;
}

// Creator

void EntityFactory::destroyEntity(entt::Registry<EntityID> &registry, EntityID entity) {
#ifdef FACTORY_DEBUG
	std::cout << "EntityFactory: destroy " << entity << std::endl;
#endif
	registry.destroy(entity);
}

std::string EntityFactory::randGroupName(std::string name) {
	std::vector<std::string> groupMembers = this->groups[name];

	int rnd = rand() % groupMembers.size();
	return groupMembers[rnd];
}

void EntityFactory::addStaticVerticalSpriteView(std::vector<SpriteView> &states, std::initializer_list<int> frames) {
	for (int n : frames) {
		states.push_back(SpriteView{sf::Vector2i(0, n)});
	}
}

void EntityFactory::setDefaultSpritesheet(StaticSpritesheet& spritesheet, Tile &tile, int y) {
	spritesheet.states["idle"] = std::vector<SpriteView>();
	int frameCount = y / tile.psize.y;
	for (int n = 0; n < frameCount; n++) {
		spritesheet.states["idle"].push_back(SpriteView{sf::Vector2i(0, n)});
	}
}

sf::Vector2f EntityFactory::caseToPixel(sf::Vector2i pos) {
	sf::Vector2f ppos = sf::Vector2f(pos) * 32.0f + 16.0f;
	return ppos;
}

// Unit
#define UNIT_FRAME_COUNT 10

void EntityFactory::setColorSwapShader(entt::Registry<EntityID> &registry, Tile &tile, EntityID playerEnt) {
#ifdef SHADER_ENABLE
	Player &player = registry.get<Player>(playerEnt);

	sf::Color col1 = sf::Color(3, 255, 205);
	sf::Color col2 = sf::Color(0, 235, 188);
	sf::Color replace1 = this->getPlayerColor(col1, player.colorIdx);
	sf::Color replace2 = this->getPlayerColor(col2, player.colorIdx);

	ShaderOptions shaderOptions;
	shaderOptions.colors["color1"] = col1;
	shaderOptions.colors["replace1"] = replace1;
	shaderOptions.colors["color2"] = col2;
	shaderOptions.colors["replace2"] = replace2;

	tile.shader = true;
	tile.shaderName = "color_swap";
	tile.shaderOptions = shaderOptions;
#else
	tile.shader = false;
#endif
}

void EntityFactory::assignSpritesheets(entt::Registry<EntityID> &registry, EntityID entity, std::string name) {
	StaticSpritesheet spritesheet;
	if (this->getXmlComponent(name, "spritesheets") && spritesheetsParser.parseStaticSpritesheets(spritesheet, this->getXmlComponent(name, "spritesheets"))) {
		registry.accomodate<StaticSpritesheet>(entity, spritesheet);
	}

	AnimatedSpritesheet animSpritesheet;
	if (this->getXmlComponent(name, "spritesheets") && spritesheetsParser.parseAnimatedSpritesheets(animSpritesheet, this->getXmlComponent(name, "spritesheets"))) {
		registry.accomodate<AnimatedSpritesheet>(entity, animSpritesheet);

		if (animSpritesheet.states.count("idle") > 0) {
			AnimatedSpriteView &view = animSpritesheet.states["idle"][0];
			Timer timer("idle", view.duration * view.frames.size(), true);
			registry.accomodate<Timer>(entity, timer);
		}

	}
}

EntityID EntityFactory::createUnit(entt::Registry<EntityID> &registry, EntityID playerEnt, std::string name, int x, int y) {
	EntityID entity = registry.create();
#ifdef FACTORY_DEBUG
	std::cout << "EntityFactory: create unit " << entity << " " << name << " at " << x << "x" << y << std::endl;
#endif
	Tile tile;
	this->parseTileFromXml(name, tile);

	tile.pos = sf::Vector2i(x, y);
	tile.ppos = this->caseToPixel(tile.pos);

	tile.view = South;

	this->setColorSwapShader(registry, tile, playerEnt);

	GameObject obj;
	this->parseGameObjectFromXml(name, obj);

	obj.player = playerEnt;
	obj.mapped = true;

	Unit unit;
	unitParser.parse(unit, this->getXmlComponent(name, "unit"));

	unit.targetEnt = 0;
	unit.nopath = 0;
	unit.destpos = tile.pos;
	unit.pathPos = tile.pos;

	registry.assign<Tile>(entity, tile);
	registry.assign<GameObject>(entity, obj);
	registry.assign<Unit>(entity, unit);

	Effects effects;
	particleEffectParser.parseEffects(effects, this->getXmlComponent(name, "effects"));
	registry.assign<Effects>(entity, effects);

	this->assignSpritesheets(registry, entity, name);

	return entity;
}

// Building
EntityID EntityFactory::startBuilding(entt::Registry<EntityID> &registry, std::string name, EntityID constructedBy) {
	EntityID entity = registry.create();
#ifdef FACTORY_DEBUG
	std::cout << "EntityFactory: start building " << entity << " " << name << " constructed by " << constructedBy << std::endl;
#endif
	Building building;
	buildingParser.parse(building, this->getXmlComponent(name, "building"));
	building.construction = 0;
	building.constructedBy = constructedBy;

	GameObject obj;
	this->parseGameObjectFromXml(name, obj);
	obj.player = 0;
	obj.mapped = false;
	obj.maxLife = obj.life;

	registry.assign<GameObject>(entity, obj);
	registry.assign<Building>(entity, building);
	return entity;
}

EntityID EntityFactory::finishBuilding(entt::Registry<EntityID> &registry, EntityID entity, EntityID playerEnt, int x, int y, bool built) {
	GameObject &obj = registry.get<GameObject>(entity);
#ifdef FACTORY_DEBUG
	std::cout << "EntityFactory: finish building " << entity << " " << obj.name << " at " << x << "x" << y << std::endl;
#endif

	obj.player = playerEnt;
	obj.mapped = built;

	Tile tile;
	this->parseTileFromXml(obj.name, tile);

	tile.pos = sf::Vector2i(x, y);
	tile.ppos = this->caseToPixel(tile.pos);
//		tile.ppos = sf::Vector2f(tile.pos) * 32.0f;

	this->setColorSwapShader(registry, tile, playerEnt);

	registry.assign<Tile>(entity, tile);

	if (!registry.has<Effects>(entity)) {
		Effects effects;
		particleEffectParser.parseEffects(effects, this->getXmlComponent(obj.name, "effects"));
		registry.assign<Effects>(entity, effects);
	}

	if (this->getXmlComponent(obj.name, "unit")) {
		Unit unit;
		unitParser.parse(unit, this->getXmlComponent(obj.name, "unit"));

		unit.targetEnt = 0;
		unit.nopath = 0;
		unit.destpos = tile.pos;

		registry.accomodate<Unit>(entity, unit);
	}

	this->assignSpritesheets(registry, entity, obj.name);

	return entity;
}

bool EntityFactory::placeBuilding(entt::Registry<EntityID> &registry, EntityID entity) {
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

EntityID EntityFactory::plantResource(entt::Registry<EntityID> &registry, std::string name, int x, int y) {
	EntityID entity = registry.create();

#ifdef FACTORY_DEBUG
	std::cout << "EntityFactory: plant resource " << entity << " " << name << " at " << x << "x" << y << std::endl;
#endif

	Tile tile;
	tile.psize = sf::Vector2f{32, 32};
	tile.size = sf::Vector2i{1, 1};

	tile.pos = sf::Vector2i(x, y);
	tile.ppos = this->caseToPixel(tile.pos);

	tile.sprite.setTexture(texManager.getRef(name));
	tile.sprite.setTextureRect(sf::IntRect(0, 0, 32, 32)); // texture need to be updated
	tile.centerRect = this->getCenterRect(name);

	Resource resource;
	resource.type = name;
	resource.level = 0;
	resource.grow = 0.0;
	resource.growRate = 0.05 + (rand() % 5) / 100.0;

	registry.assign<Tile>(entity, tile);
	registry.assign<Resource>(entity, resource);

	return entity;
}

EntityID EntityFactory::growedResource(entt::Registry<EntityID> &registry, std::string name, EntityID entity) {
	std::string rname = this->randGroupName(name);
	Tile &oldTile = registry.get<Tile>(entity);

	Tile tile;
	this->parseTileFromXml(rname, tile);

	tile.pos = oldTile.pos;
	tile.ppos = this->caseToPixel(tile.pos);

	tile.sprite.setTexture(texManager.getRef(rname));
	tile.sprite.setTextureRect(sf::IntRect(0, 0, tile.psize.x, tile.psize.y)); // texture need to be updated
	tile.centerRect = this->getCenterRect(rname);

	registry.remove<Tile>(entity);
	registry.assign<Tile>(entity, tile);

	Resource &resource = registry.get<Resource>(entity);
	this->parseResourceFromXml(rname, resource);

	Effects effects;
	effects.effects["spend"] = name + "_spend";
	registry.assign<Effects>(entity, effects);

	this->assignSpritesheets(registry, entity, rname);

	return entity;
}

EntityID EntityFactory::createParticleEffect(entt::Registry<EntityID> &registry, std::string name, ParticleEffectOptions options) {
	EntityID entity = registry.create();

#ifdef FACTORY_DEBUG
	std::cout << "EntityFactory: create map effect " << entity << " " << name << std::endl;
#endif
	ParticleEffect effect;

	options.texMgr = &texManager;

	particleEffectParser.parse(effect, this->getXmlComponent(name, "particle"), options);

	effect.pos = effect.spawner->center;
	if (options.hasDestPos())
		effect.destpos = options.destPos;
	else
		effect.destpos = effect.pos;

	registry.assign<ParticleEffect>(entity, effect);

	Effects effects;
	particleEffectParser.parseEffects(effects, this->getXmlComponent(name, "effects"));
	registry.assign<Effects>(entity, effects);

	return entity;
}

EntityID EntityFactory::createDecor(entt::Registry<EntityID> &registry, std::string name, int x, int y) {
	std::string rname = this->randGroupName(name);

	EntityID entity = registry.create();
#ifdef FACTORY_DEBUG
	std::cout << "EntityFactory: create decor " << entity << " " << rname << " at " << x << "x" << y << std::endl;
#endif
	Tile tile;
	this->parseTileFromXml(rname, tile);

	tile.pos = sf::Vector2i(x, y);
	tile.ppos = this->caseToPixel(tile.pos);

	Decor decor;
	this->parseDecorFromXml(rname, decor);

	registry.assign<Tile>(entity, tile);
	registry.assign<Decor>(entity, decor);

	this->assignSpritesheets(registry, entity, rname);

	return entity;
}

// Player
EntityID EntityFactory::createPlayer(entt::Registry<EntityID> &registry, std::string team, bool ai) {
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
		player.resourceType = "nature";
	else if (team == "neonaz")
		player.resourceType = "pollution";

	registry.assign<Player>(entity, player);
	return entity;
}

EntityID EntityFactory::createTimer(entt::Registry<EntityID> &registry, EntityID emitterEnt, std::string name, float duration, bool loop) {
	EntityID entity = registry.create();
	Timer timer(emitterEnt, name, duration, loop);
	registry.assign<Timer>(entity, timer);
	return entity;
}

void EntityFactory::loadManifest(std::string filename) {
#ifdef FACTORY_DEBUG
	std::cout << "EntityFactory: load manifest " << filename << std::endl;
#endif
	tinyxml2::XMLDocument doc;
	doc.LoadFile(filename.c_str());

	for (tinyxml2::XMLElement *childEl : doc.RootElement()) {
		std::string name = childEl->Name();
		std::string sfilename = childEl->Attribute("path");

		tinyxml2::XMLDocument *sdoc = new tinyxml2::XMLDocument();
		sdoc->LoadFile(sfilename.c_str());

		std::string entName = sdoc->RootElement()->Attribute("name");

		this->loadedXmlDocs[entName] = sdoc;

		if (sdoc->RootElement()->Attribute("group")) {
			std::string entGroup = sdoc->RootElement()->Attribute("group");
			if (this->groups.count(entGroup) == 0) {
				this->groups[entGroup] = std::vector<std::string>();
				this->groups[entGroup].push_back(entName);
			} else {
				this->groups[entGroup].push_back(entName);
			}

			if (this->groupCount.count(entGroup) == 0) {
				this->groupCount[entGroup] = 1;
			} else {
				this->groupCount[entGroup] = this->groupCount[entGroup] + 1;
			}
		}

		texLoader.parse(sdoc->RootElement());
		sndLoader.parse(sdoc->RootElement());
	}
#ifdef FACTORY_DEBUG
	std::cout << "EntityFactory: manifest loaded " << filename << std::endl;
#endif
}

void EntityFactory::load() {
	if (!this->loaded) {
		this->loadManifest("defs/manifest.xml");

		this->loadTerrains();

		this->loadPlayerColors("defs/unit_color.xml");

		this->loadMisc();
		this->loadTechTrees();
		this->loadDecorGenerator("defs/dec/decor_generator.xml");
		this->loaded = true;
	}
}

EntityFactory::EntityFactory() {
	texLoader.setManager(&texManager);
	sndLoader.setManager(&sndManager);
	this->loaded = false;
}
