#pragma once

#include "Game.hpp"

#include "Stages/GameStage.hpp"
#include "Stages/GameOver.hpp"

#include "Map.hpp"

#include "Components/Components.hpp"

#include "Systems/GameSystem.hpp"
#include "Systems/GameGeneratorSystem.hpp"
#include "Systems/TileAnimSystem.hpp"
#include "Systems/MapLayersSystem.hpp"
#include "Systems/DrawMapSystem.hpp"
#include "Systems/MinimapSystem.hpp"
#include "Systems/ResourcesSystem.hpp"
#include "Systems/ConstructionSystem.hpp"
#include "Systems/PathfindingSystem.hpp"
#include "Systems/SteeringSystem.hpp"
#include "Systems/CombatSystem.hpp"
#include "Systems/VictorySystem.hpp"
#include "Systems/SoundSystem.hpp"
#include "Systems/FxSystem.hpp"
#include "Systems/InterfaceSystem.hpp"
#include "Systems/DeletionSystem.hpp"

#include "AI.hpp"


class GameEngine : public GameSystem, public GameStage {
public:
	EntityID emptyEntity;
	float timePerTick;
	float currentTime;
	unsigned long ticks;
	bool markUpdateLayer;
	int gameSpeed;

	GameGeneratorSystem gameGenerator;
	ResourcesSystem resources;
	TileAnimSystem tileAnim;
	DrawMapSystem drawMap;
	MinimapSystem minimap;
	MapLayersSystem mapLayers;
	ConstructionSystem construction;
	PathfindingSystem pathfinding;
	SteeringSystem steering;
	CombatSystem combat;
	VictorySystem victory;
	SoundSystem sound;
	FxSystem fx;
	DeletionSystem deletion;
	AI ai;

	InterfaceSystem interface;

	sf::View gameView;
	sf::View guiView;

	float zoomLevel;

	GameEngine(Game *game, unsigned int mapWidth, unsigned int mapHeight, std::string playerTeam);

	~GameEngine();

	void init();

	void centerMapView(sf::Vector2i position);

	void reset();

	void fadeOutCallback();

	void setVaults(GameVault *vault);
	// scale for other resolutions than 800x600
// scale for other resolutions than 800x600
	inline float scaleX() const {
		return this->width / 800.0;
	}
	inline float scaleY() const {
		return this->height / 600.0;
	}

	void generate(unsigned int mapWidth, unsigned int mapHeight, std::string playerTeam);

	void debugGui(float dt);

	void updatePlayers(float dt);
	void updateHundred(float dt);
	void updateDecade(float dt);
	void updateEveryFrame(float dt);

	sf::IntRect viewClip();

	void debugDraw(float dt);
	void draw(float dt);

	void setGameSpeed(float factor) ;

	void update(float dt);
	void updateMoveView(float dt);

	void handleEvent(sf::Event & event);

	// signals
	void receive(const GameStageChange &event);

};

