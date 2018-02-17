#include <iostream>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include <third_party/imgui/imgui.h>
#include <third_party/imgui/imgui-sfml.h>
#include <third_party/imgui/ImguiWindowsFileIO.hpp>
#include "imgui-custom.h"

#include <third_party/Particles/ParticleSystem.h>

#include "third_party/tinyxml2.h"
#include "third_party/tixml2ex.h"


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



static std::map<ParticleSystemMode, std::string> revPartSysModes =
{
	{ ParticleSystemMode::Points, "points" },
	{ ParticleSystemMode::Texture, "texture" },
	{ ParticleSystemMode::Spritesheet, "spritesheet" },
	{ ParticleSystemMode::AnimatedSpritesheet, "animated_spritesheet" },
	{ ParticleSystemMode::Metaball, "metaball" },
};

static std::map<SpawnerMode, std::string> revSpawnModes =
{
	{ SpawnerMode::Point, "point" },
	{ SpawnerMode::Box, "box" },
	{ SpawnerMode::Circle, "circle" },
	{ SpawnerMode::Disk, "disk" },
};

static std::map<VelocityGeneratorMode, std::string> revVelGenModes =
{
	{ VelocityGeneratorMode::Angled, "angled" },
	{ VelocityGeneratorMode::Vector, "vector" },
	{ VelocityGeneratorMode::Aimed, "aimed" },
};


const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

const int MAX_PARTICLE_COUNT = 100000;

void initParticleSystem();
void setSpawnMode();
void setVelocityGeneratorMode();
void gui();

// Particle System
particles::ParticleSystem *particleSystem;

// Particle Spawner
particles::ParticleSpawner *spawner;

// Particle Generators
particles::ParticleGenerator *velocityGenerator;
particles::TimeGenerator *timeGenerator;
particles::RotationGenerator *rotationGenerator;
particles::ColorGenerator *colorGenerator;
particles::SizeGenerator *sizeGenerator;

// Particle Updaters
particles::TimeUpdater *timeUpdater;
particles::RotationUpdater *rotationUpdater;
particles::ColorUpdater *colorUpdater;
particles::SizeUpdater *sizeUpdater;
particles::EulerUpdater *eulerUpdater;

// Global State
ParticleSystemMode particleSystemMode = ParticleSystemMode::Texture;
SpawnerMode spawnerMode = SpawnerMode::Point;
VelocityGeneratorMode velocityGeneratorMode = VelocityGeneratorMode::Angled;

sf::Texture *circleTexture;
sf::Texture *blobTexture;
sf::Texture *starTexture;
sf::Texture *spritesheetTexture;

void initParticleSystem() {
	if (particleSystem) delete particleSystem;

	switch (particleSystemMode) {
	case ParticleSystemMode::Points:
		particleSystem = new particles::PointParticleSystem(MAX_PARTICLE_COUNT);
		break;
	case ParticleSystemMode::Spritesheet:
	case ParticleSystemMode::AnimatedSpritesheet:
		particleSystem = new particles::SpriteSheetParticleSystem(MAX_PARTICLE_COUNT, spritesheetTexture);
		break;
	case ParticleSystemMode::Metaball:
		particleSystem = new particles::MetaballParticleSystem(MAX_PARTICLE_COUNT, blobTexture, WINDOW_WIDTH, WINDOW_HEIGHT);
		break;
	case ParticleSystemMode::Texture:
	default:
		particleSystem = new particles::TextureParticleSystem(MAX_PARTICLE_COUNT, circleTexture);
		break;
	}

	particleSystem->emitRate = 160.f;

	setSpawnMode();
	setVelocityGeneratorMode();

	timeGenerator = particleSystem->addGenerator<particles::TimeGenerator>();
	timeGenerator->minTime = 1.f;
	timeGenerator->maxTime = 5.f;

	colorGenerator = particleSystem->addGenerator<particles::ColorGenerator>();
	colorGenerator->minStartCol = sf::Color(16, 124, 167, 255);
	colorGenerator->maxStartCol = sf::Color(30, 150, 255, 255);
	colorGenerator->minEndCol = sf::Color(57, 0, 150, 0);
	colorGenerator->maxEndCol = sf::Color(235, 128, 220, 0);

	sizeGenerator = particleSystem->addGenerator<particles::SizeGenerator>();
	sizeGenerator->minStartSize = 20.f;
	sizeGenerator->maxStartSize = 60.f;
	sizeGenerator->minEndSize = 10.f;
	sizeGenerator->maxEndSize = 30.f;

	rotationGenerator = particleSystem->addGenerator<particles::RotationGenerator>();
	rotationGenerator->minStartAngle = -20.f;
	rotationGenerator->maxStartAngle = -20.f;
	rotationGenerator->minEndAngle = 90.f;
	rotationGenerator->maxEndAngle = 90.f;

	timeUpdater = particleSystem->addUpdater<particles::TimeUpdater>();
	colorUpdater = particleSystem->addUpdater<particles::ColorUpdater>();
	sizeUpdater = particleSystem->addUpdater<particles::SizeUpdater>();
	rotationUpdater = particleSystem->addUpdater<particles::RotationUpdater>();
	eulerUpdater = particleSystem->addUpdater<particles::EulerUpdater>();

	if (particleSystemMode == ParticleSystemMode::Spritesheet) {
		auto texCoordGen = particleSystem->addGenerator<particles::TexCoordsRandomGenerator>();
		texCoordGen->texCoords.push_back(sf::IntRect(0, 0, 8, 8));
		texCoordGen->texCoords.push_back(sf::IntRect(8, 0, 8, 8));
		texCoordGen->texCoords.push_back(sf::IntRect(16, 0, 8, 8));
		texCoordGen->texCoords.push_back(sf::IntRect(24, 0, 8, 8));
	}
	else if (particleSystemMode == ParticleSystemMode::AnimatedSpritesheet) {
		auto texCoordGen = particleSystem->addGenerator<particles::TexCoordsGenerator>();
		texCoordGen->texCoords = sf::IntRect(0, 0, 8, 8);

		auto animationUpdater = particleSystem->addUpdater<particles::AnimationUpdater>();
		animationUpdater->frames.push_back(sf::IntRect(0, 0, 8, 8));
		animationUpdater->frames.push_back(sf::IntRect(8, 0, 8, 8));
		animationUpdater->frames.push_back(sf::IntRect(16, 0, 8, 8));
		animationUpdater->frames.push_back(sf::IntRect(24, 0, 8, 8));

		animationUpdater->frameTime = 0.8f;
		animationUpdater->looped = true;
	}
	else if (particleSystemMode == ParticleSystemMode::Metaball) {
		auto ps = dynamic_cast<particles::MetaballParticleSystem *>(particleSystem);
		ps->color = sf::Color(20, 50, 100, 255);
	}
}

void setSpawnMode() {
	if (spawner) particleSystem->removeSpawner(spawner);

	switch (spawnerMode) {
	case SpawnerMode::Box: {
		auto boxSpawner = particleSystem->addSpawner<particles::BoxSpawner>();
		boxSpawner->size = sf::Vector2f(160.f, 160.f);
		spawner = boxSpawner;
	}
	break;

	case SpawnerMode::Circle: {
		auto circleSpawner = particleSystem->addSpawner<particles::CircleSpawner>();
		circleSpawner->radius = sf::Vector2f(70.f, 40.f);
		spawner = circleSpawner;
	}
	break;

	case SpawnerMode::Disk: {
		auto diskSpawner = particleSystem->addSpawner<particles::DiskSpawner>();
		diskSpawner->radius = sf::Vector2f(70.f, 40.f);
		spawner = diskSpawner;
	}
	break;

	case SpawnerMode::Point:
	default: {
		spawner = particleSystem->addSpawner<particles::PointSpawner>();
	}
	break;
	}
}

void setVelocityGeneratorMode() {
	if (velocityGenerator) particleSystem->removeGenerator(velocityGenerator);

	switch (velocityGeneratorMode) {
	case VelocityGeneratorMode::Vector: {
		auto vectorGenerator = particleSystem->addGenerator<particles::VectorVelocityGenerator>();
		vectorGenerator->minStartVel = sf::Vector2f(20.f, -20.f);
		vectorGenerator->maxStartVel = sf::Vector2f(40.f, -40.f);
		velocityGenerator = vectorGenerator;
	}
	break;

	case VelocityGeneratorMode::Aimed: {
		auto aimedGenerator = particleSystem->addGenerator<particles::AimedVelocityGenerator>();
		aimedGenerator->goal = sf::Vector2f(0.5f * WINDOW_WIDTH, 0.5f * WINDOW_HEIGHT);
		aimedGenerator->minStartSpeed = 100.f;
		aimedGenerator->maxStartSpeed = 150.f;
		velocityGenerator = aimedGenerator;
	}
	break;

	case VelocityGeneratorMode::Angled:
	default: {
		auto angledGenerator = particleSystem->addGenerator<particles::AngledVelocityGenerator>();
		angledGenerator->minAngle = -20.f;
		angledGenerator->maxAngle = 20.f;
		angledGenerator->minStartSpeed = 100.f;
		angledGenerator->maxStartSpeed = 150.f;
		velocityGenerator = angledGenerator;
	}
	break;
	}
}

void save(std::string filename) {
	tinyxml2::XMLDocument xmlDoc;
	tinyxml2::XMLNode * pRoot = xmlDoc.NewElement("entity");
	xmlDoc.InsertFirstChild(pRoot);

	tinyxml2::XMLElement * pElement = xmlDoc.NewElement("particle");
	pElement->SetAttribute("name", "noname");
	pElement->SetAttribute("count", int(particleSystem->emitRate));
	pElement->SetAttribute("max", int(MAX_PARTICLE_COUNT));
	pElement->SetAttribute("type", revPartSysModes[particleSystemMode].c_str());
	switch (particleSystemMode) {
	case ParticleSystemMode::Metaball:
		auto ps = dynamic_cast<particles::MetaballParticleSystem *>(particleSystem);

		pElement->SetAttribute("a", ps->color.a);
		pElement->SetAttribute("r", ps->color.r);
		pElement->SetAttribute("g", ps->color.g);
		pElement->SetAttribute("b", ps->color.b);
		break;
	}

	// spawner
	tinyxml2::XMLElement * spawnerElement = xmlDoc.NewElement("spawner");
	spawnerElement->SetAttribute("type", revSpawnModes[spawnerMode].c_str());
	if (spawnerMode == SpawnerMode::Point) {
		// nop
	}
	else if (spawnerMode == SpawnerMode::Box) {
		auto sp = dynamic_cast<particles::BoxSpawner *>(spawner);
		spawnerElement->SetAttribute("x", sp->size.x);
		spawnerElement->SetAttribute("y", sp->size.y);
	}
	else if (spawnerMode == SpawnerMode::Circle) {
		auto sp = dynamic_cast<particles::CircleSpawner *>(spawner);
		spawnerElement->SetAttribute("x", sp->radius.x);
		spawnerElement->SetAttribute("y", sp->radius.y);
	}
	else if (spawnerMode == SpawnerMode::Disk) {
		auto sp = dynamic_cast<particles::DiskSpawner *>(spawner);
		spawnerElement->SetAttribute("x", sp->radius.x);
		spawnerElement->SetAttribute("y", sp->radius.y);
	}

	// time generator
	tinyxml2::XMLElement * timeGenElement = xmlDoc.NewElement("time_generator");
	timeGenElement->SetAttribute("min_time", timeGenerator->minTime);
	timeGenElement->SetAttribute("max_time", timeGenerator->maxTime);

	// size generator
	tinyxml2::XMLElement * sizeGenElement = xmlDoc.NewElement("size_generator");
	sizeGenElement->SetAttribute("min_start_size", sizeGenerator->minStartSize);
	sizeGenElement->SetAttribute("max_start_size", sizeGenerator->maxStartSize);
	sizeGenElement->SetAttribute("min_end_size", sizeGenerator->minEndSize);
	sizeGenElement->SetAttribute("max_end_size", sizeGenerator->maxEndSize);

	// velocity generator
	tinyxml2::XMLElement * velGenElement = xmlDoc.NewElement("velocity_generator");

	velGenElement->SetAttribute("type", revVelGenModes[velocityGeneratorMode].c_str());

	if (velocityGeneratorMode == VelocityGeneratorMode::Angled) {
		auto gen = dynamic_cast<particles::AngledVelocityGenerator *>(velocityGenerator);
		velGenElement->SetAttribute("min_angle", gen->minAngle);
		velGenElement->SetAttribute("max_angle", gen->maxAngle);
		velGenElement->SetAttribute("min_start_speed", gen->minStartSpeed);
		velGenElement->SetAttribute("max_start_speed", gen->maxStartSpeed);
	}
	else if (velocityGeneratorMode == VelocityGeneratorMode::Vector) {
		auto gen = dynamic_cast<particles::VectorVelocityGenerator *>(velocityGenerator);
		velGenElement->SetAttribute("min_start_vel_x", gen->minStartVel.x);
		velGenElement->SetAttribute("min_start_vel_y", gen->minStartVel.y);
		velGenElement->SetAttribute("max_start_vel_x", gen->maxStartVel.x);
		velGenElement->SetAttribute("max_start_vel_y", gen->maxStartVel.y);
	}
	else if (velocityGeneratorMode == VelocityGeneratorMode::Aimed) {
		auto gen = dynamic_cast<particles::AimedVelocityGenerator *>(velocityGenerator);
		velGenElement->SetAttribute("min_start_speed", gen->minStartSpeed);
		velGenElement->SetAttribute("max_start_speed", gen->maxStartSpeed);
	}

	pElement->InsertEndChild(spawnerElement);
	pElement->InsertEndChild(timeGenElement);
	pElement->InsertEndChild(sizeGenElement);
	pElement->InsertEndChild(velGenElement);

	if (particleSystemMode != ParticleSystemMode::Points) {
		tinyxml2::XMLElement * rotGenElement = xmlDoc.NewElement("rotation_generator");
		rotGenElement->SetAttribute("min_start_angle", rotationGenerator->minStartAngle);
		rotGenElement->SetAttribute("max_start_angle", rotationGenerator->maxStartAngle);
		rotGenElement->SetAttribute("min_end_angle", rotationGenerator->minEndAngle);
		rotGenElement->SetAttribute("max_end_angle", rotationGenerator->maxEndAngle);
		pElement->InsertEndChild(rotGenElement);

		tinyxml2::XMLElement * eulerUpElement = xmlDoc.NewElement("euler_updater");
		eulerUpElement->SetAttribute("accel_x", eulerUpdater->globalAcceleration.x);
		eulerUpElement->SetAttribute("accel_y", eulerUpdater->globalAcceleration.y);
		pElement->InsertEndChild(eulerUpElement);

	}

	if (particleSystemMode != ParticleSystemMode::Metaball) {

// color generator
		tinyxml2::XMLElement * colGenElement = xmlDoc.NewElement("color_generator");

		tinyxml2::XMLElement * minSCol = xmlDoc.NewElement("min_start_col");
		tinyxml2::XMLElement * maxSCol = xmlDoc.NewElement("max_start_col");
		tinyxml2::XMLElement * minECol = xmlDoc.NewElement("min_end_col");
		tinyxml2::XMLElement * maxECol = xmlDoc.NewElement("max_end_col");

		minSCol->SetAttribute("a", colorGenerator->minStartCol.a);
		minSCol->SetAttribute("r", colorGenerator->minStartCol.r);
		minSCol->SetAttribute("g", colorGenerator->minStartCol.g);
		minSCol->SetAttribute("b", colorGenerator->minStartCol.b);
		colGenElement->InsertEndChild(minSCol);

		maxSCol->SetAttribute("a", colorGenerator->maxStartCol.a);
		maxSCol->SetAttribute("r", colorGenerator->maxStartCol.r);
		maxSCol->SetAttribute("g", colorGenerator->maxStartCol.g);
		maxSCol->SetAttribute("b", colorGenerator->maxStartCol.b);
		colGenElement->InsertEndChild(maxSCol);

		minECol->SetAttribute("a", colorGenerator->minEndCol.a);
		minECol->SetAttribute("r", colorGenerator->minEndCol.r);
		minECol->SetAttribute("g", colorGenerator->minEndCol.g);
		minECol->SetAttribute("b", colorGenerator->minEndCol.b);
		colGenElement->InsertEndChild(minECol);

		maxECol->SetAttribute("a", colorGenerator->maxEndCol.a);
		maxECol->SetAttribute("r", colorGenerator->maxEndCol.r);
		maxECol->SetAttribute("g", colorGenerator->maxEndCol.g);
		maxECol->SetAttribute("b", colorGenerator->maxEndCol.b);
		colGenElement->InsertEndChild(maxECol);

		pElement->InsertEndChild(colGenElement);

	}

	pRoot->InsertEndChild(pElement);
	xmlDoc.SaveFile(filename.c_str());
}


sf::Color parseColor(tinyxml2::XMLElement *element) {
	return sf::Color(element->IntAttribute("r"), element->IntAttribute("g"), element->IntAttribute("b"), element->IntAttribute("a"));
}

void load(std::string filename) {
	tinyxml2::XMLDocument xmlDoc;
	xmlDoc.LoadFile(filename.c_str());
	tinyxml2::XMLElement *element = xmlDoc.RootElement()->FirstChildElement("particle");
	tinyxml2::XMLElement * particleEl = element;

	if (particleEl) {
		sf::Vector2f spriteSize(0, 0);

		tinyxml2::XMLElement * psizeEl = element->FirstChildElement("psize");
		if (psizeEl)
			spriteSize = sf::Vector2f{(float)psizeEl->IntAttribute("x"), (float)psizeEl->IntAttribute("y")};

		particleSystemMode = partSysModes[particleEl->Attribute("type")];

		initParticleSystem();
		particleSystem->emitRate = particleEl->IntAttribute("count");

		switch (particleSystemMode) {
		case ParticleSystemMode::Points:
			break;
		case ParticleSystemMode::Texture:
			break;
		case ParticleSystemMode::Spritesheet: {
//			auto texCoordGen = spriteSystem->addGenerator<particles::TexCoordsGenerator>();
//			texCoordGen->texCoords = sf::IntRect(options.direction * spriteSize.x, 0, spriteSize.x, spriteSize.y);
		}
		break;
		case ParticleSystemMode::AnimatedSpritesheet:
			break;
		case ParticleSystemMode::Metaball: {
			// FIXME size == screen size
			auto metaball = dynamic_cast<particles::MetaballParticleSystem *>(particleSystem);
			metaball->color = parseColor(particleEl);
		}
		break;
		default:
			break;
		}

		tinyxml2::XMLElement *spawnerEl = particleEl->FirstChildElement("spawner");

		spawnerMode = spawnModes[spawnerEl->Attribute("type")];
		setSpawnMode();

		switch (spawnerMode) {
		case SpawnerMode::Point:
			break;
		case SpawnerMode::Box: {
			auto boxSpawner = dynamic_cast<particles::BoxSpawner *>(spawner);
			sf::Vector2f size(spawnerEl->FloatAttribute("x"), spawnerEl->FloatAttribute("y"));
			boxSpawner->size = size;
		}
		break;
		case SpawnerMode::Circle: {
			auto circleSpawner = dynamic_cast<particles::CircleSpawner *>(spawner);
			sf::Vector2f radius(spawnerEl->FloatAttribute("x"), spawnerEl->FloatAttribute("y"));
			circleSpawner->radius = radius;
		}
		break;
		case SpawnerMode::Disk: {
			auto diskSpawner = dynamic_cast<particles::DiskSpawner *>(spawner);
			sf::Vector2f radius(spawnerEl->FloatAttribute("x"), spawnerEl->FloatAttribute("y"));
			diskSpawner->radius = radius;
		}
		break;
		default:
			break;
		}

		tinyxml2::XMLElement *timeGenEl = particleEl->FirstChildElement("time_generator");

		timeGenerator->minTime = timeGenEl->FloatAttribute("min_time");
		timeGenerator->maxTime = timeGenEl->FloatAttribute("max_time");

		tinyxml2::XMLElement *sizeGenEl = particleEl->FirstChildElement("size_generator");

		if (sizeGenEl) {
			sizeGenerator->minStartSize = sizeGenEl->FloatAttribute("min_start_size");
			sizeGenerator->maxStartSize = sizeGenEl->FloatAttribute("max_start_size");
			sizeGenerator->minEndSize = sizeGenEl->FloatAttribute("min_end_size");
			sizeGenerator->maxEndSize = sizeGenEl->FloatAttribute("max_end_size");
		}

		tinyxml2::XMLElement *velGenEl = particleEl->FirstChildElement("velocity_generator");

		velocityGeneratorMode = velGenModes[velGenEl->Attribute("type")];
		setVelocityGeneratorMode();

		switch (velocityGeneratorMode) {
		case VelocityGeneratorMode::Vector:
		{
			auto vectorGenerator = dynamic_cast<particles::VectorVelocityGenerator *>(velocityGenerator);
			vectorGenerator->minStartVel = sf::Vector2f(velGenEl->FloatAttribute("min_start_vel_x"), velGenEl->FloatAttribute("min_start_vel_y"));
			vectorGenerator->maxStartVel = sf::Vector2f(velGenEl->FloatAttribute("max_start_vel_x"), velGenEl->FloatAttribute("max_start_vel_y"));
		}
		break;
		case VelocityGeneratorMode::Angled:
		{
			auto angledGenerator = dynamic_cast<particles::AngledVelocityGenerator *>(velocityGenerator);
			angledGenerator->minAngle = velGenEl->FloatAttribute("min_angle");
			angledGenerator->maxAngle = velGenEl->FloatAttribute("max_angle");
			angledGenerator->minStartSpeed = velGenEl->FloatAttribute("min_start_speed");
			angledGenerator->maxStartSpeed = velGenEl->FloatAttribute("max_start_speed");
		}
		break;
		case VelocityGeneratorMode::Aimed:
		{
			auto aimedGenerator = dynamic_cast<particles::AimedVelocityGenerator *>(velocityGenerator);
//			aimedGenerator->goal = options.destPos;
			aimedGenerator->minStartSpeed = velGenEl->FloatAttribute("min_start_speed");
			aimedGenerator->maxStartSpeed = velGenEl->FloatAttribute("max_start_speed");
		}
		break;
		default:
			break;
		}

		tinyxml2::XMLElement *rotGenEl = particleEl->FirstChildElement("rotation_generator");
		if (rotGenEl) {
			rotationGenerator->minStartAngle = rotGenEl->FloatAttribute("min_start_angle");
			rotationGenerator->maxStartAngle = rotGenEl->FloatAttribute("max_start_angle");
			rotationGenerator->minEndAngle = rotGenEl->FloatAttribute("min_end_angle");
			rotationGenerator->maxEndAngle = rotGenEl->FloatAttribute("max_end_angle");
		}

		tinyxml2::XMLElement *colGenEl = particleEl->FirstChildElement("color_generator");

		colorGenerator->minStartCol = parseColor(colGenEl->FirstChildElement("min_start_col"));
		colorGenerator->maxStartCol = parseColor(colGenEl->FirstChildElement("max_start_col"));
		colorGenerator->minEndCol = parseColor(colGenEl->FirstChildElement("min_end_col"));
		colorGenerator->maxEndCol = parseColor(colGenEl->FirstChildElement("max_end_col"));

		tinyxml2::XMLElement *eulUpEl = particleEl->FirstChildElement("euler_updater");
		if (eulUpEl) {
			eulerUpdater->globalAcceleration = sf::Vector2f(eulUpEl->FloatAttribute("accel_x"), eulUpEl->FloatAttribute("accel_y"));
		}

	}
}

std::vector<std::string> recentFiles;
bool showSave = false;
bool saveCurrent = true;
std::string currentFile = "";
bool showLoad = false;

void saveGui() {
	if (showSave) {
		std::string save_file;
		if ( fileIOWindow( save_file, recentFiles, "Save", {"*.xml", "*.*"} ) )
		{
			showSave = false;			
			if ( !save_file.empty() )
			{
				currentFile = save_file;
				recentFiles.push_back( save_file );
				save(save_file);
			}
		}
	}
}

void loadGui() {
	if ( showLoad )
	{
		std::string open_file;
		if ( fileIOWindow( open_file, recentFiles, "Open", {"*.xml", "*.*"}, true  ) )
		{
			showLoad = false;
			if ( !open_file.empty() )
			{
				currentFile = open_file;
				recentFiles.push_back( open_file );
				load( open_file );
			}
		}
	}
}


void gui() {
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open")) {
				showLoad = true;
			}
			if (ImGui::MenuItem("Save")) {
				if(currentFile.empty()) {
					showSave = true;
				} else {
					save(currentFile);
				}
			}
			if (ImGui::MenuItem("Save as")) {
				showSave = true;
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Quit")) {
				exit(1);
			}
			ImGui::EndMenu();
		}
		/*
		        if (ImGui::BeginMenu("Edit"))
		        {
		            if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
		            if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
		            ImGui::Separator();
		            if (ImGui::MenuItem("Cut", "CTRL+X")) {}
		            if (ImGui::MenuItem("Copy", "CTRL+C")) {}
		            if (ImGui::MenuItem("Paste", "CTRL+V")) {}
		            ImGui::EndMenu();
		        }
		        */
		ImGui::EndMainMenuBar();
	}

	saveGui();
	loadGui();

	ImGui::SetNextWindowSize(ImVec2(380, 630), ImGuiSetCond_FirstUseEver);
	ImGui::Begin("Particles Editor");

	if (ImGui::CollapsingHeader("Particle System", ImGuiTreeNodeFlags_DefaultOpen)) {
		const char* particleSystemItems[] = { "Points", "Textured", "Spritesheet", "Animated Spritesheet", "Metaball" };
		static int particleSystemItem = 1;
		if (ImGui::Combo("Render Mode", &particleSystemItem, particleSystemItems, 5)) {
			particleSystemMode = static_cast<ParticleSystemMode>(particleSystemItem);
			initParticleSystem();
		}

//		ImGui::SliderFloat("emit rate", &particleSystem->emitRate, 0.f, 1500.f);
		ImGui::InputFloat("emit rate", &particleSystem->emitRate, 1.0f, 1500.0f);

		if (particleSystemMode == ParticleSystemMode::Texture || particleSystemMode == ParticleSystemMode::Spritesheet || particleSystemMode == ParticleSystemMode::AnimatedSpritesheet) {
			auto ps = dynamic_cast<particles::TextureParticleSystem *>(particleSystem);
			ImGui::Checkbox("Additive blending", &ps->additiveBlendMode);
		}

		if (particleSystemMode == ParticleSystemMode::Texture) {
			auto ps = dynamic_cast<particles::TextureParticleSystem *>(particleSystem);

			const char* textureItems[] = { "Circle", "Blob", "Star" };
			static int textureItem = 0;
			if (ImGui::Combo("Texture", &textureItem, textureItems, 3)) {
				if (textureItem == 0) {
					ps->setTexture(circleTexture);
				}
				else if (textureItem == 1) {
					ps->setTexture(blobTexture);
				}
				if (textureItem == 2) {
					ps->setTexture(starTexture);
				}
			}
		}

		if (particleSystemMode == ParticleSystemMode::Metaball) {
			auto ps = dynamic_cast<particles::MetaballParticleSystem *>(particleSystem);
			ImGui::ColorEdit("Color", &ps->color);
			ImGui::SliderFloat("Threshold", &ps->threshold, 0.f, 0.999f);
		}
	}

	if (ImGui::CollapsingHeader("Particle Spawner")) {
		const char* spawnItems[] = { "Point", "Box", "Circle", "Disk" };
		static int spawnItem = 0;
		if (ImGui::Combo("Spawner Mode", &spawnItem, spawnItems, 4)) {
			spawnerMode = static_cast<SpawnerMode>(spawnItem);
			setSpawnMode();
		}

		if (spawnerMode == SpawnerMode::Point) {
			// nop
		}
		else if (spawnerMode == SpawnerMode::Box) {
			auto sp = dynamic_cast<particles::BoxSpawner *>(spawner);
			ImGui::SliderFloat2("size", &sp->size, 0.f, 500.f);
//            ImGui::DragFloatRange2("size", &sp->size.x, &sp->size.y, 0.5f, 0.0f, 500.0f, "X: %.1f", "Y: %.1f");
		}
		else if (spawnerMode == SpawnerMode::Circle) {
			auto sp = dynamic_cast<particles::CircleSpawner *>(spawner);
			ImGui::SliderFloat2("size", &sp->radius, 0.f, 500.f);
//            ImGui::DragFloatRange2("size", &sp->radius.x, &sp->radius.y, 0.5f, 0.0f, 500.0f, "X: %.1f", "Y: %.1f");
		}
		else if (spawnerMode == SpawnerMode::Disk) {
			auto sp = dynamic_cast<particles::DiskSpawner *>(spawner);
			ImGui::SliderFloat2("size", &sp->radius, 0.f, 500.f);
//            ImGui::DragFloatRange2("size", &sp->radius.x, &sp->radius.y, 0.5f, 0.0f, 500.0f, "X: %.1f", "Y: %.1f");
		}
	}

	if (ImGui::CollapsingHeader("Velocity Generator")) {
		const char* velItems[] = { "Angled", "Vector", "Aimed" };
		static int velItem = 0;
		if (ImGui::Combo("Velocity Mode", &velItem, velItems, 3)) {
			velocityGeneratorMode = static_cast<VelocityGeneratorMode>(velItem);
			setVelocityGeneratorMode();
		}

		if (velocityGeneratorMode == VelocityGeneratorMode::Angled) {
			auto gen = dynamic_cast<particles::AngledVelocityGenerator *>(velocityGenerator);
			ImGui::SliderFloat("min angle", &gen->minAngle, -180.f, 180.f);
			ImGui::SliderFloat("max angle", &gen->maxAngle, -180.f, 180.f);
			ImGui::SliderFloat("min speed", &gen->minStartSpeed, 0.f, 500.f);
			ImGui::SliderFloat("max speed", &gen->maxStartSpeed, 0.f, 500.f);
		}
		else if (velocityGeneratorMode == VelocityGeneratorMode::Vector) {
			auto gen = dynamic_cast<particles::VectorVelocityGenerator *>(velocityGenerator);
			ImGui::SliderFloat2("min", &gen->minStartVel, 0.f, 500.f);
			ImGui::SliderFloat2("max", &gen->maxStartVel, 0.f, 500.f);
		}
		else if (velocityGeneratorMode == VelocityGeneratorMode::Aimed) {
			auto gen = dynamic_cast<particles::AimedVelocityGenerator *>(velocityGenerator);
			ImGui::SliderFloat2("min", &gen->goal, 0.f, 500.f);
			ImGui::SliderFloat("min speed", &gen->minStartSpeed, 0.f, 500.f);
			ImGui::SliderFloat("max speed", &gen->maxStartSpeed, 0.f, 500.f);
		}
	}

	if (ImGui::CollapsingHeader("Time Generator")) {
		ImGui::SliderFloat("min", &timeGenerator->minTime, 0.f, 60.f);
		ImGui::SliderFloat("max", &timeGenerator->maxTime, 0.f, 60.f);
	}

	if (particleSystemMode != ParticleSystemMode::Points && ImGui::CollapsingHeader("Size Generator")) {
		ImGui::SliderFloat("min start size", &sizeGenerator->minStartSize, 0.f, 100.f);
		ImGui::SliderFloat("max start size", &sizeGenerator->maxStartSize, 0.f, 100.f);
		ImGui::SliderFloat("min end size", &sizeGenerator->minEndSize, 0.f, 100.f);
		ImGui::SliderFloat("max end size", &sizeGenerator->maxEndSize, 0.f, 100.f);
	}

	if (particleSystemMode != ParticleSystemMode::Points && ImGui::CollapsingHeader("Rotation Generator")) {
		ImGui::SliderFloat("min start angle", &rotationGenerator->minStartAngle, -180.f, 180.f);
		ImGui::SliderFloat("max start angle", &rotationGenerator->maxStartAngle, -180.f, 180.f);
		ImGui::SliderFloat("min end angle", &rotationGenerator->minEndAngle, -180.f, 180.f);
		ImGui::SliderFloat("max end angle", &rotationGenerator->maxEndAngle, -180.f, 180.f);
	}

	if (particleSystemMode != ParticleSystemMode::Metaball && ImGui::CollapsingHeader("Color Generator")) {
		ImGui::ColorEdit("min start", &colorGenerator->minStartCol);
		ImGui::ColorEdit("max start", &colorGenerator->maxStartCol);
		ImGui::ColorEdit("min end", &colorGenerator->minEndCol);
		ImGui::ColorEdit("max end", &colorGenerator->maxEndCol);
	}

	if (ImGui::CollapsingHeader("Euler Updater")) {
		ImGui::SliderFloat2("gravity", &eulerUpdater->globalAcceleration, 0.f, 200.f);
	}

	ImGui::End();
}


int main() {
	circleTexture = new sf::Texture();
	blobTexture = new sf::Texture();
	starTexture = new sf::Texture();
	spritesheetTexture = new sf::Texture();
	circleTexture->loadFromFile("medias/extra/circleTexture.png");
	blobTexture->loadFromFile("medias/extra/blobTexture.png");
	starTexture->loadFromFile("medias/extra/starTexture.png");
	spritesheetTexture->loadFromFile("medias/extra/spritesheetTexture.png");
	circleTexture->setSmooth(true);
	blobTexture->setSmooth(true);
	starTexture->setSmooth(true);
	spritesheetTexture->setSmooth(false);

	initParticleSystem();

	sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Particles Editor");
	window.setVerticalSyncEnabled(true);

	ImGui::SFML::Init(window);

	sf::Clock clock;
	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			ImGui::SFML::ProcessEvent(event);

			if (event.type == sf::Event::Closed ||
			        (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)) {
				window.close();
			}
		}

		sf::Vector2i mouse = sf::Mouse::getPosition(window);
		sf::Vector2f pos = window.mapPixelToCoords(mouse);

		spawner->center = pos;

		sf::Time dt = clock.restart();
		ImGui::SFML::Update(window, dt);
		particleSystem->update(dt);

		gui();

		window.clear();

		particleSystem->render(window);
		ImGui::SFML::Render(window);

		window.display();
	}

	ImGui::SFML::Shutdown();
	delete particleSystem;
}