#pragma once

#include "Game.hpp"

#include "Stages/GameStage.hpp"
#include "Stages/GameOver.hpp"

#include "Map.hpp"

#include "Components.hpp"

#include "Systems/GameSystem.hpp"
#include "Systems/GameGeneratorSystem.hpp"
#include "Systems/TileAnimSystem.hpp"
#include "Systems/MapLayersSystem.hpp"
#include "Systems/DrawMapSystem.hpp"
#include "Systems/MinimapSystem.hpp"
#include "Systems/ResourcesSystem.hpp"
#include "Systems/ConstructionSystem.hpp"
#include "Systems/PathfindingSystem.hpp"
#include "Systems/CombatSystem.hpp"
#include "Systems/VictorySystem.hpp"
#include "Systems/SoundSystem.hpp"
#include "Systems/FxSystem.hpp"

#include "AI.hpp"

enum class Action {
	None,
	Selecting,
	Building
};

enum class MoveView {
	DontMove,
	MoveWest,
	MoveEast,
	MoveNorth,
	MoveSouth
};

class GameEngine : public GameSystem, public GameStage {
public:
	GameGeneratorSystem gameGenerator;
	ResourcesSystem resources;
	TileAnimSystem tileAnim;
	DrawMapSystem drawMap;
	MinimapSystem minimap;
	MapLayersSystem mapLayers;
	ConstructionSystem construction;
	PathfindingSystem pathfinding;
	CombatSystem combat;
	VictorySystem victory;
	SoundSystem sound;
	FxSystem fx;

	AI ai;

	sf::Sprite iface;
	sf::Sprite box;
	int box_w;
	sf::Sprite minimap_bg;
	int minimap_bg_h;
	sf::Sprite indice_bg;
	sf::Sprite indice;

	Action action;
	sf::Vector2f selectionStart;
	sf::Vector2f selectionEnd;

	std::vector<EntityID> selectedObjs;

	EntityID emptyEntity;

	EntityID currentBuild;
	std::string currentBuildType;

	EntityID currentPlayer;

	sf::View gameView;
	sf::View guiView;

	float timePerTick;
	float currentTime;
	unsigned long ticks;
	bool markUpdateLayer;

	int debugCorner;
	int gameSpeed;

	MoveView moveView;

	EntityID selectedDebugObj;
	bool showDebugWindow;

	float zoomLevel;

	GameEngine(Game *game, unsigned int mapWidth, unsigned int mapHeight, std::string playerTeam) {
		this->game = game;
		this->init();
		this->generate(mapWidth, mapHeight, playerTeam);
		this->moveView = MoveView::DontMove;
		this->zoomLevel = 1.0;
	}

	~GameEngine() {
		this->setGameSpeed(1.0);
		// FIXME: registry must actually resides in GameEngine instead of game
		this->vault->registry.reset();
		delete this->map;
	}

	void init() {
		this->action = Action::None;
		this->timePerTick = 0.1;
		this->currentTime = 0.0;
		this->ticks = 0;
		this->currentBuild = 0;
		this->markUpdateLayer = false;

		this->currentPlayer = 0;
		this->map = new Map();

		this->setSize(this->game->width, this->game->height);
		this->setVaults(&(this->game->vault));

		text.setFont(this->vault->factory.fntManager.getRef("samos"));
		text.setCharacterSize(48);
#if SFML_VERSION_MAJOR==2 && SFML_VERSION_MINOR > 3
		text.setFillColor(sf::Color::White);
#else
// SFML 2.3
		text.setColor(sf::Color::White);
#endif

		this->initView();

#ifdef GAME_ENGINE_DEBUG
		std::cout << "GameEngine: loading ..." << std::endl;
#endif

		// show loading screen
		this->game->window.clear(sf::Color::Black);
		this->text.setString("LOADING");
		sf::FloatRect textRect = this->text.getLocalBounds();
		text.setOrigin(textRect.left + textRect.width / 2.0f,
		               textRect.top  + textRect.height / 2.0f);
		text.setPosition(sf::Vector2f(this->width / 2, this->height / 2));
		this->game->window.draw(this->text);
		this->game->window.display();

		this->vault->factory.load();

#ifdef GAME_ENGINE_DEBUG
		std::cout << "GameEngine: loaded !" << std::endl;
#endif
		this->initEffects();
		this->fadeIn();
		this->gameSpeed = 1;

		this->debugCorner = 1;
		this->showDebugWindow = false;
		this->selectedDebugObj = 0;
	}

	void reset() {
		this->fadeIn();
		this->nextStage = 0;
	}

	void fadeOutCallback() {
		switch (this->nextStage)
		{
		case NextStageStr("play_menu"):
			this->game->pushRegisteredStage("play_menu");
			break;
		case NextStageStr("game_over"): {
			this->game->pushRegisteredStage("game_over");
		}
		break;
		}
	}

	void setVaults(GameVault *vault) {
		emptyEntity = vault->registry.create(); // create Entity 0 as special empty Entity
		this->setVault(vault);

		// set shared systems
		gameGenerator.setShared(vault, this->map, this->width, this->height);
		tileAnim.setShared(vault, this->map, this->width, this->height);
		resources.setShared(vault, this->map, this->width, this->height);
		drawMap.setShared(vault, this->map, this->width, this->height);
		minimap.setShared(vault, this->map, this->width, this->height);
		mapLayers.setShared(vault, this->map, this->width, this->height);
		construction.setShared(vault, this->map, this->width, this->height);
		pathfinding.setShared(vault, this->map, this->width, this->height);
		combat.setShared(vault, this->map, this->width, this->height);
		victory.setShared(vault, this->map, this->width, this->height);
		sound.setShared(vault, this->map, this->width, this->height);
		fx.setShared(vault, this->map, this->width, this->height);
		ai.setShared(vault, this->map, this->width, this->height);
	}

	void initView() {
		sf::Vector2f pos = sf::Vector2f(this->game->window.getSize());
		guiView.setSize(pos);
		gameView.setSize(pos);
		pos *= 0.5f;
		guiView.setCenter(pos);
		gameView.setCenter(pos);
	}

	void centerMapView(sf::Vector2i position) {
		sf::Vector2f centre(position.x * 32, position.y * 32);
		this->gameView.setCenter(centre);
	}

	// scale for other resolutions than 800x600
	float scaleX() {
		return this->width / 800.0;
	}
	float scaleY() {
		return this->height / 600.0;
	}

	void generate(unsigned int mapWidth, unsigned int mapHeight, std::string playerTeam) {
		mapLayers.initTileMaps();
		mapLayers.initTransitions();

		this->currentPlayer = gameGenerator.generate(mapWidth, mapHeight, playerTeam);

		mapLayers.updateAllTransitions();

		ai.generate();

		Player &player = this->vault->registry.get<Player>(this->currentPlayer);
		if (player.team != "neutral") {
			this->centerMapView(player.initialPos);
			iface.setTexture(this->vault->factory.getTex("interface_" + player.team));
			box.setTexture(this->vault->factory.getTex("box_" + player.team));
			box_w = this->vault->factory.getTex("box_" + player.team).getSize().x;
			minimap_bg.setTexture(this->vault->factory.getTex("minimap_" + player.team));
			minimap_bg_h = this->vault->factory.getTex("minimap_" + player.team).getSize().y;
			indice.setTexture(this->vault->factory.getTex("indice_" + player.team));
			indice_bg.setTexture(this->vault->factory.getTex("indice_bg_" + player.team));
		} else {
			this->centerMapView(sf::Vector2i(this->map->width / 2, this->map->height / 2));
		}

		mapLayers.initCorpses();

		minimap.init(sf::Vector2f(this->scaleX() * 10, this->scaleY() * (600 - 123 + 14)), 96.0 * this->scaleX());
		victory.init();
		pathfinding.init();
	}

	void menuGui() {
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // Transparent background
		if (ImGui::Begin("Menu", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
		{

			if (ImGui::ImageButtonAnim(this->vault->factory.texManager.getRef("menu_button"),
			                           this->vault->factory.texManager.getRef("menu_button"),
			                           this->vault->factory.texManager.getRef("menu_button_down"))) {
				nextStage = NextStageStr("play_menu");
				fadeOut();
			}
			ImGui::End();
		}
		ImGui::PopStyleColor();
	}

	void gameStateGui() {
		Player &player = this->vault->registry.get<Player>(this->currentPlayer);
		float leftDist = 192.0f * this->scaleX();
		float topDist = 8.0f * this->scaleY();

		ImVec2 window_pos = ImVec2(leftDist, topDist);
		ImVec2 window_pos_pivot = ImVec2(0.0f, 0.0f);
		ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
//		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // Transparent background
		if (ImGui::Begin("State", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
		{
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0, 255, 0, 255));
			ImGui::ProgressBar((float)player.resources / victory.resourcesVictory(), ImVec2(200.0f * this->scaleX(), 0.0f), "");
			ImGui::PopStyleColor();

			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(255, 0, 0, 255));
			ImGui::ProgressBar((float)player.butchery / victory.butcheryVictory(), ImVec2(200.0f * this->scaleX(), 0.0f), "");
			ImGui::PopStyleColor();

			ImGui::End();
		}
//		ImGui::PopStyleColor();
	}

	void constructionProgressGui(EntityID consEnt) {
		if (consEnt && this->vault->registry.valid(consEnt)) {
			Building &buildingCons = this->vault->registry.get<Building>(consEnt);
			GameObject &objCons = this->vault->registry.get<GameObject>(consEnt);

			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0, 255, 0, 255));
			ImGui::ProgressBar((buildingCons.maxBuildTime - buildingCons.buildTime) / buildingCons.maxBuildTime, ImVec2(110.0f * this->scaleX(), 0.0f), "");
			ImGui::PopStyleColor(); ImGui::SameLine();

			if (buildingCons.buildTime > 0) {
				ImGui::Image(this->vault->factory.texManager.getRef(objCons.name + "_icon_building")); ImGui::SameLine();
			} else {
				if (ImGui::ImageButtonAnim(this->vault->factory.texManager.getRef(objCons.name + "_icon_built"),
				                           this->vault->factory.texManager.getRef(objCons.name + "_icon_built"),
				                           this->vault->factory.texManager.getRef(objCons.name + "_icon_built_down"))) {
//					std::cout << "built clicked " << std::endl;
					this->action = Action::Building;
					this->currentBuild = this->vault->factory.finishBuilding(this->vault->registry, consEnt, this->currentPlayer, 8, 8, false);
				}
				ImGui::SameLine();
			}
		}
	}

	void actionGui() {
		Player &player = this->vault->registry.get<Player>(this->currentPlayer);
		if (player.team != "neutral") {
			float uiX = 590.0f * this->scaleX();
			float uiHeight = 124.0f * this->scaleY();
			float uiWidth = 250.0f * this->scaleX();
			float uiLWidth = 300.0f * this->scaleX();
			float uiRWidth = 200.0f * this->scaleX();

			ImVec2 window_pos = ImVec2(uiX, ImGui::GetIO().DisplaySize.y - uiHeight);
			ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImVec2(uiWidth, uiHeight), ImGuiCond_Always);
			this->guiPushStyles();
			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // Transparent background
			if (ImGui::Begin("Actions", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
			{
				if (this->selectedObjs.size() == 1) {
					EntityID selectedObj = this->selectedObjs[0];

					Tile &tile = this->vault->registry.get<Tile>(selectedObj);
					GameObject &obj = this->vault->registry.get<GameObject>(selectedObj);

					if (this->vault->registry.has<Building>(selectedObj)) {
						Building &building = this->vault->registry.get<Building>(selectedObj);
						this->constructionProgressGui(building.construction);
					}

					if (this->vault->registry.has<Unit>(selectedObj)) {
						Unit &unit = this->vault->registry.get<Unit>(selectedObj);

						if (this->vault->factory.texManager.hasRef(obj.name + "_face")) {
							ImGui::BeginGroup();
							ImGui::Image(this->vault->factory.texManager.getRef(obj.name + "_face"));
							ImGui::EndGroup(); ImGui::SameLine();
						}
						ImGui::BeginGroup();
						ImGui::Text("PV: %d", (int)obj.life);
						ImGui::Text("AC: %d", unit.attack1.power);
						ImGui::Text("AE: %d", unit.attack2.power);
						ImGui::Text("DE: %d", unit.attack2.distance);
						ImGui::EndGroup();
					}

					if (this->vault->registry.has<Unit>(selectedObj)) {
						Unit &unit = this->vault->registry.get<Unit>(selectedObj);

						ImGui::BeginGroup();
						if (ImGui::ImageButtonAnim(this->vault->factory.texManager.getRef(player.team + "_move"),
						                           this->vault->factory.texManager.getRef(player.team + "_move"),
						                           this->vault->factory.texManager.getRef(player.team + "_move_down"))) {
							std::cout << "TODO: move clicked " << std::endl;
						}

						ImGui::SameLine();
						if (ImGui::ImageButtonAnim(this->vault->factory.texManager.getRef(player.team + "_attack"),
						                           this->vault->factory.texManager.getRef(player.team + "_attack"),
						                           this->vault->factory.texManager.getRef(player.team + "_attack_down"))) {
							std::cout << "TODO: attack clicked " << std::endl;
						}
						ImGui::EndGroup();
					}

					if (this->vault->registry.has<Building>(selectedObj)) {

						Building &building = this->vault->registry.get<Building>(selectedObj);

						TechNode *pnode = this->vault->factory.getTechNode(player.team, obj.name);
						if (building.construction) {
							if (ImGui::ImageButtonAnim(this->vault->factory.texManager.getRef(player.team + "_cancel"),
							                           this->vault->factory.texManager.getRef(player.team + "_cancel"),
							                           this->vault->factory.texManager.getRef(player.team + "_cancel_down"))) {
								this->vault->factory.destroyEntity(this->vault->registry, building.construction);
								building.construction = 0;
							}
						} else {
							if (pnode->children.size() > 0) {
								int buts = 0;
								for (TechNode &node : pnode->children) {
									if (ImGui::ImageButtonAnim(this->vault->factory.texManager.getRef(node.type + "_icon"),
									                           this->vault->factory.texManager.getRef(node.type + "_icon"),
									                           this->vault->factory.texManager.getRef(node.type + "_icon_down"))) {
										switch (node.comp) {
										case TechComponent::Building: {
											if (!building.construction) {
												EntityID newConsEnt = this->vault->factory.startBuilding(this->vault->registry, node.type, selectedObj);
												// need to reload the parent building to assign construction
												Building &pBuilding = this->vault->registry.get<Building>(selectedObj);
												pBuilding.construction = newConsEnt;
											}
										}
										break;
										case TechComponent::Character:
											if (this->trainUnit(node.type, this->currentPlayer, selectedObj)) {
												this->markUpdateLayer = true;
											}
											break;
										case TechComponent::Resource:
											this->seedResources(player.resourceType, selectedObj);
											break;
										}
									}
									if (buts % 3 != 2)
										ImGui::SameLine();
									buts++;
								}
							}
						}
					}
				} else {
					if (this->selectedObjs.size() == 0) {
						this->constructionProgressGui(player.rootConstruction);

						if (player.rootConstruction) {
							if (ImGui::ImageButtonAnim(this->vault->factory.texManager.getRef(player.team + "_cancel"),
							                           this->vault->factory.texManager.getRef(player.team + "_cancel"),
							                           this->vault->factory.texManager.getRef(player.team + "_cancel_down"))) {

								this->vault->factory.destroyEntity(this->vault->registry, player.rootConstruction);
								player.rootConstruction = 0;
							}
						} else {
							TechNode *node = this->vault->factory.getTechRoot(player.team);
							if (ImGui::ImageButtonAnim(this->vault->factory.texManager.getRef(node->type + "_icon"),
							                           this->vault->factory.texManager.getRef(node->type + "_icon"),
							                           this->vault->factory.texManager.getRef(node->type + "_icon_down"))) {
								if (!player.rootConstruction)
									player.rootConstruction = this->vault->factory.startBuilding(this->vault->registry, node->type, 0);
							}
						}
					}
				}
				ImGui::End();
			}
			ImGui::PopStyleColor();
			this->guiPopStyles();
		}
	}

	void debugGui(float dt) {
		sf::Vector2f gamePos = (this->game->window.mapPixelToCoords(sf::Mouse::getPosition(this->game->window), this->gameView));
		sf::Vector2f gameMapPos = gamePos;
		gameMapPos.x /= 32.0;
		gameMapPos.y /= 32.0;

		const float DISTANCE = 10.0f;
		ImVec2 window_pos = ImVec2((debugCorner & 1) ? ImGui::GetIO().DisplaySize.x - DISTANCE : DISTANCE, (debugCorner & 2) ? ImGui::GetIO().DisplaySize.y - DISTANCE : DISTANCE);
		ImVec2 window_pos_pivot = ImVec2((debugCorner & 1) ? 1.0f : 0.0f, (debugCorner & 2) ? 1.0f : 0.0f);
		ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.3f)); // Transparent background
		ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[2]);
		if (ImGui::Begin("Debug", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
		{
			ImGui::Text("FPS: %.2f", 1 / dt);
			ImGui::Text("Total Entities: %d", (int)this->vault->registry.size());
			ImGui::Text("Drawn Entities: %d", (int)drawMap.entitiesDrawList.size());
			ImGui::Checkbox("Debug layer", &drawMap.showDebugLayer);

			ImGui::Text("Speed"); ImGui::SameLine();
			ImGui::RadioButton("0", &gameSpeed, 0); ImGui::SameLine();
			ImGui::RadioButton("x1", &gameSpeed, 1); ImGui::SameLine();
			ImGui::RadioButton("x4", &gameSpeed, 4); ImGui::SameLine();
			ImGui::RadioButton("x16", &gameSpeed, 16);

			this->setGameSpeed(gameSpeed);

			if (this->selectedDebugObj) {
				EntityID selectedObj = this->selectedDebugObj;
				if (this->vault->registry.valid(selectedObj)) {
					Tile &tile = this->vault->registry.get<Tile>(selectedObj);
					ImGui::Text("ID: %d", selectedObj);
					ImGui::Text("Case position: %dx%d", tile.pos.x, tile.pos.y);
					ImGui::Text("Case size: %dx%d", tile.size.x, tile.size.y);
					ImGui::Text("Pixel position: %.2fx%.2f", tile.ppos.x, tile.ppos.y);
					ImGui::Text("Pixel size: %.2fx%.2f", tile.psize.x, tile.psize.y);
					ImGui::Text("Offset: %dx%d", tile.offset.x, tile.offset.y);
					ImGui::Text("Z: %d", tile.z);
					ImGui::Text("Center: %dx%d:%dx%d", tile.centerRect.left, tile.centerRect.top, tile.centerRect.width, tile.centerRect.height);
					ImGui::Text("View: %d", tile.view);
					ImGui::Text("State: %s", tile.state.c_str());
//					ImGui::Text("Current frame: %d", tile.animHandlers[tile.state].getCurrentFrame());
//					ImGui::Text("Current anim: %d", tile.animHandlers[tile.state].getCurrentColumn());

					if (this->vault->registry.has<GameObject>(selectedObj)) {
						GameObject &obj = this->vault->registry.get<GameObject>(selectedObj);
						ImGui::Separator();
						ImGui::Text("GameObject: ");
						ImGui::Text("Name: %s", obj.name.c_str());
						ImGui::Text("Team: %s", obj.team.c_str());
						ImGui::Text("Life: %f", obj.life);
					}
					if (this->vault->registry.has<Unit>(selectedObj)) {
						Unit &unit = this->vault->registry.get<Unit>(selectedObj);
						ImGui::Separator();
						ImGui::Text("Unit: ");
						ImGui::Text("Next pos: %dx%d", unit.nextpos.x, unit.nextpos.y);
						ImGui::Text("Dest pos: %dx%d", unit.destpos.x, unit.destpos.y);
						ImGui::Text("Dest attack: %d", (int)unit.destAttack);
					}
					if (this->vault->registry.has<Resource>(selectedObj)) {
						Resource &resource = this->vault->registry.get<Resource>(selectedObj);
						ImGui::Separator();
						ImGui::Text("Resource: ");
						ImGui::Text("Level: %d\n", resource.level);
						ImGui::Text("Grow: %.2f\n", resource.grow);
					}
				}
			}

			ImGui::Separator();
			ImGui::Text("Mouse Position: (%.1f,%.1f)", ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y);
			ImGui::Text("Game Position: (%.1f,%.1f)", gameMapPos.x, gameMapPos.y);

			if (ImGui::BeginPopupContextWindow())
			{
				if (ImGui::MenuItem("Top-left", NULL, debugCorner == 0)) debugCorner = 0;
				if (ImGui::MenuItem("Top-right", NULL, debugCorner == 1)) debugCorner = 1;
				if (ImGui::MenuItem("Bottom-left", NULL, debugCorner == 2)) debugCorner = 2;
				if (ImGui::MenuItem("Bottom-right", NULL, debugCorner == 3)) debugCorner = 3;
				ImGui::EndPopup();
			}
			ImGui::End();
		}
		ImGui::PopStyleColor();
		ImGui::PopFont();
	}

	void updatePlayers(float dt) {
		auto playerView = this->vault->registry.view<Player>();

		for (EntityID entity : playerView) {
			Player &player = playerView.get(entity);
			player.objsByType.clear();
		}

		auto objView = this->vault->registry.view<Tile, GameObject>();
		for (EntityID entity : objView) {
			GameObject &obj = objView.get<GameObject>(entity);

			if (obj.player)
			{
				Player &player = this->vault->registry.get<Player>(obj.player);
				if (player.objsByType.count(obj.name)) {
					player.objsByType[obj.name].push_back(entity);
				} else {
					std::vector<EntityID> vec;
					vec.push_back(entity);
					player.objsByType[obj.name] = vec;
				}
			}
		}
	}

// remove entity from selected is not valid anymore
	void updateSelected(float dt) {
		std::vector<EntityID> newSelectedObjs;
		for (EntityID entity : this->selectedObjs) {
			if (this->vault->registry.valid(entity))
				newSelectedObjs.push_back(entity);
		}
		this->selectedObjs = newSelectedObjs;
	}

	void updateHundred(float dt) {
		auto playerView = this->vault->registry.view<Player>();
		for (EntityID entity : playerView) {
			Player &player = playerView.get(entity);

			int playerObjs = 0;
			for (auto pair : player.objsByType) {
				playerObjs += pair.second.size();
			}
#ifdef GAME_ENGINE_DEBUG
			std::cout << "Player stats: " << entity << " " << player.team << " objs:" << playerObjs << " resources:" << player.resources << " butchery:" << player.butchery << std::endl;
#endif
			if (victory.checkVictoryConditions(entity)) {
#ifdef GAME_ENGINE_DEBUG
				std::cout << "Player: " << entity << " WINS !" << std::endl;
#endif
				GameOver *go = (GameOver *)this->game->getStage("game_over");

				if (entity == this->currentPlayer) {
					go->win = true;
				} else {
					go->win = false;
				}

				go->player = player;

				nextStage = NextStageStr("game_over");
				fadeOut();
			}
		}
	}

	void updateDecade(float dt) {
		victory.updateStats(dt);
		victory.updatePlayerBonus(this->currentPlayer);
		victory.clearStats();

		minimap.update(this->currentPlayer, dt);
		combat.updateFront(dt);
	}

	void updateEveryFrame(float dt)
	{
		if (this->markUpdateLayer) {
			this->mapLayers.updateObjsLayer(0);
			this->markUpdateLayer = false;
		}

		this->tileAnim.update(dt);
		this->pathfinding.updateMovement(dt);

		this->sound.update(dt);
		this->sound.cleanPlaying(dt);
	}

	sf::IntRect viewClip() {
		sf::View wview = this->game->window.getView();
		sf::FloatRect screenRect(sf::Vector2f(wview.getCenter().x - (wview.getSize().x) / 2, wview.getCenter().y - (wview.getSize().y) / 2) , wview.getSize());

		int mx = screenRect.left / 32.0;
		int my = screenRect.top / 32.0;
		int mw = mx + screenRect.width / 32.0;
		int mh = my + screenRect.height / 32.0;

		mx = mx < 0 ? 0 : mx;
		my = my < 0 ? 0 : my;
		mw = mw > this->map->width ? this->map->width : mw;
		mh = mh > this->map->height ? this->map->height : mh;

		return sf::IntRect(sf::Vector2i(mx, my), sf::Vector2i(mw - mx, mh - my));
	}

	void draw(float dt) {
		sf::IntRect clip = this->viewClip();

		mapLayers.drawTerrainTileMap(this->game->window, dt);
		drawMap.draw(this->game->window, clip, dt);
		if (this->gameSpeed < 2)
			fx.draw(this->game->window, clip, dt);

		mapLayers.drawFogTileMap(this->game->window, dt);

		// draw selected
		for (EntityID selectedObj : this->selectedObjs) {
			//if (this->vault->registry.valid(selectedObj))
			{
				Tile &tile = this->vault->registry.get<Tile>(selectedObj);

				sf::Vector2f pos;
				pos.x = tile.ppos.x - (tile.centerRect.left + tile.centerRect.width / 2) + 16 + tile.offset.x * 32;
				pos.y = tile.ppos.y - (tile.centerRect.top + tile.centerRect.height / 2) + 16 + tile.offset.y * 32;

				sf::Sprite selected(this->vault->factory.getTex("selected"));
				selected.setTextureRect(sf::IntRect(0, 0, 7, 7));
				selected.setPosition(pos);
				this->game->window.draw(selected);

				selected.setTextureRect(sf::IntRect(0, 7, 7, 7));
				selected.setPosition(sf::Vector2f(pos.x + tile.psize.x - 7, pos.y));
				this->game->window.draw(selected);

				selected.setTextureRect(sf::IntRect(0, 14, 7, 7));
				selected.setPosition(sf::Vector2f(pos.x + tile.psize.x - 7, pos.y + tile.psize.y - 7));
				this->game->window.draw(selected);

				selected.setTextureRect(sf::IntRect(0, 21, 7, 7));
				selected.setPosition(sf::Vector2f(pos.x, pos.y + tile.psize.y - 7));
				this->game->window.draw(selected);
			}
		}

		if (this->showDebugWindow && this->selectedDebugObj) {
			if (this->vault->registry.valid(this->selectedDebugObj)) {
				Tile &tile = this->vault->registry.get<Tile>(this->selectedDebugObj);

				// draw surface case
				for (sf::Vector2i const &p : this->tileSurface(tile)) {
					sf::RectangleShape srect;

					sf::Vector2f pos;
					pos.x = p.x * 32;
					pos.y = p.y * 32;

					srect.setSize(sf::Vector2f(32, 32));
					srect.setFillColor(sf::Color(0x00, 0x00, 0x00, 0x00));
					srect.setOutlineColor(sf::Color(0x00, 0xFF, 0xFF, 0xFF));
					srect.setOutlineThickness(2);
					srect.setPosition(pos);

					this->game->window.draw(srect);
				}

				// draw tile case
				sf::RectangleShape trect;
				sf::Vector2f pos;
				pos.x = tile.pos.x * 32;
				pos.y = tile.pos.y * 32;
				trect.setSize(sf::Vector2f(32, 32));
				trect.setFillColor(sf::Color(0x00, 0x00, 0x00, 0x00));
				trect.setOutlineColor(sf::Color::Blue);
				trect.setOutlineThickness(2);
				trect.setPosition(pos);
				this->game->window.draw(trect);

				// draw pixel rect
				sf::RectangleShape drect;
				sf::Vector2f dpos = this->tileDrawPosition(tile);
				drect.setSize(tile.psize);
				drect.setFillColor(sf::Color(0x00, 0x00, 0x00, 0x00));
				drect.setOutlineColor(sf::Color(0xff, 0xb6, 0xc1, 0xff));
				drect.setOutlineThickness(2);
				drect.setPosition(dpos);
				this->game->window.draw(drect);

				// draw center rect
				sf::RectangleShape crect;
				sf::Vector2f cpos;
				cpos.x = dpos.x + tile.centerRect.left;
				cpos.y = dpos.y + tile.centerRect.top;
				crect.setSize(sf::Vector2f(tile.centerRect.width, tile.centerRect.height));
				crect.setFillColor(sf::Color(0x00, 0x00, 0x00, 0x00));
				crect.setOutlineColor(sf::Color(255, 36, 196));
				crect.setOutlineThickness(2);
				crect.setPosition(cpos);
				this->game->window.draw(crect);
			} else {
				this->selectedDebugObj = 0;
			}

		}

		if (this->action == Action::Selecting) {
			sf::RectangleShape rectangle;

			sf::Vector2f pos;

			rectangle.setSize(this->selectionEnd - this->selectionStart);
			rectangle.setFillColor(sf::Color(0x00, 0x00, 0x00, 0x00));
			rectangle.setOutlineColor(sf::Color::Blue);
			rectangle.setOutlineThickness(2);
			rectangle.setPosition(this->selectionStart);

			this->game->window.draw(rectangle);
		}

		if (this->currentBuild) {
//			Tile &tile = this->vault->registry.get<Tile>(this->currentBuild);
			std::vector<sf::Vector2i> restricted = this->canBuild(this->currentPlayer, this->currentBuild);
			sf::Sprite forbid(this->vault->factory.getTex("forbid"));
			forbid.setTextureRect(sf::IntRect(0, 0, 20, 20));
			for (sf::Vector2i const &p : restricted) {
				sf::Vector2f sp(p.x * 32, p.y * 32);
				forbid.setPosition(sp);
				this->game->window.draw(forbid);
			}
		}

		this->game->window.setView(this->guiView);

		iface.setPosition(sf::Vector2f(0, 0));
		iface.setScale(this->scaleX(), this->scaleY());
		this->game->window.draw(iface);

		minimap_bg.setPosition(sf::Vector2f(0, (600 - minimap_bg_h) * this->scaleY()));
		minimap_bg.setScale(this->scaleX(), this->scaleY());
		this->game->window.draw(minimap_bg);

		box.setPosition(sf::Vector2f((800 - box_w) * this->scaleX(), (600 - 136) * this->scaleY()));
		box.setScale(this->scaleX(), this->scaleY());
		this->game->window.draw(box);

		if (this->showDebugWindow)
			this->debugGui(dt);
		this->menuGui();
		this->gameStateGui();
		this->actionGui();

		victory.draw(this->game->window, dt);
		/*
				indice_bg.setPosition(sf::Vector2f((this->width - indice_bg.getTexture()->getSize().x)/2.0, 0));
				indice_bg.setScale(this->width / 800.0, this->height / 600.0);
				this->game->window.draw(indice_bg);
		*/
		ImGui::SFML::Render(this->game->window);
		/*
				indice.setPosition(sf::Vector2f((this->width - indice.getTexture()->getSize().x)/2.0, 0));
				indice.setScale(this->width / 800.0, this->height / 600.0);
				this->game->window.draw(indice);
		*/

		minimap.draw(this->game->window, dt);
		minimap.drawClip(this->game->window, this->gameView, clip, dt);

		this->updateFading();
	}

	void setGameSpeed(float factor) {
		if (factor == 0.0) {
			this->game->window.setFramerateLimit(30);
			this->timePerTick = FLT_MAX;
		} else {
			this->game->window.setFramerateLimit(30 * factor);
			this->timePerTick = 0.1 / factor;
		}
	}

	void destroyObjs(float dt) {
		auto objView = this->vault->registry.persistent<Tile, GameObject>();
		for (EntityID entity : objView) {
			Tile &tile = objView.get<Tile>(entity);
			GameObject &obj = objView.get<GameObject>(entity);

			if (obj.destroy) {
				if (this->vault->registry.has<Unit>(entity)) {
					EntityID corpseEnt = mapLayers.getTile(obj.name + "_corpse_" + std::to_string(obj.player), 0);
//					std::cout << "GameEngine: set corpse " << obj.name + "_corpse" << " " << corpseEnt << " at " << tile.pos.x << " " << tile.pos.y << std::endl;
					this->map->corpses.set(tile.pos.x, tile.pos.y, corpseEnt);
				}
				if (this->vault->registry.has<Building>(entity)) {
					Building &building = this->vault->registry.get<Building>(entity);
					this->map->corpses.set(tile.pos.x, tile.pos.y, mapLayers.getTile("ruin", 0));
					if (building.construction) {
						// destroy currently building cons
						this->vault->factory.destroyEntity(this->vault->registry, building.construction);
					}
				}
				this->vault->factory.destroyEntity(this->vault->registry, entity);
			}
		}
	}

	void update(float dt) {
	}

	void update(sf::Time &elapsed) {
		float dt = elapsed.asSeconds();

		float updateDt = dt;
		this->game->window.setView(this->gameView);

		if (this->timePerTick == FLT_MAX) return;

		this->updateEveryFrame(dt);
		this->updateMoveView(dt);

		if (this->gameSpeed < 2) {
			this->fx.update(elapsed);
		}
		else {
			this->fx.clear();
		}

		this->currentTime += dt;

		if (this->currentTime < this->timePerTick) return;

		this->ticks++;
		updateDt = this->currentTime;
		this->currentTime = 0.0;

		if (this->ticks % 100 == 0) {
			this->updateHundred(updateDt * 100);
		}

		if (this->ticks % 10 == 0) {
			this->updateDecade(updateDt * 10);
		}

		this->pathfinding.update(dt);

		this->updatePlayers(updateDt);

		this->construction.update(updateDt);

		this->combat.update(updateDt);
		this->destroyObjs(updateDt);

		this->resources.update(updateDt);
		this->mapLayers.update(updateDt);

		this->mapLayers.updateSpectatorFog(this->currentPlayer, dt);
		this->mapLayers.updatePlayerFogLayer(this->currentPlayer, sf::IntRect(0, 0, this->map->width, this->map->height), dt);

		ai.update(updateDt);

		this->updateSelected(updateDt);

		sf::Vector2f viewPos = this->gameView.getCenter();
//		std::cout << "GameEngine: set listener position to " << viewPos.x / 32.0 << "x" << viewPos.y / 32.0 << std::endl;
		sf::Listener::setPosition(viewPos.x / 32.0, 0.f, viewPos.y / 32.0);
	}

	void updateMoveView(float dt) {
		sf::Vector2f center = this->gameView.getCenter();
		float mov = 160.0 * dt;
		switch (this->moveView) {
		case MoveView::DontMove:
			break;
		case MoveView::MoveWest:
			if (center.x > 128)
				this->gameView.move(sf::Vector2f{ -mov, 0.0});
			break;
		case MoveView::MoveEast:
			if (center.x < this->map->width * 32 - 128)
				this->gameView.move(sf::Vector2f{mov, 0.0});
			break;
		case MoveView::MoveNorth:
			if (center.y > 128)
				this->gameView.move(sf::Vector2f{ 0.0, -mov});
			break;
		case MoveView::MoveSouth:
			if (center.y < this->map->height * 32 - 128)
				this->gameView.move(sf::Vector2f{0.0, mov});
			break;
		}
	}

	void clearSelection() {
		if (this->action == Action::Selecting) {
			this->selectionStart = sf::Vector2f(0, 0);
			this->selectionEnd = sf::Vector2f(0, 0);
			this->action = Action::None;
		}
	}

	void orderSelected(sf::Vector2f destpos) {
		if (this->selectedObjs.size() > 0) {
			EntityID destEnt = this->ennemyAtPosition(this->currentPlayer, destpos.x, destpos.y);

			if (destEnt) {
				int curObj = 0;
				while (curObj < this->selectedObjs.size()) {
					EntityID selectedObj = this->selectedObjs[curObj];
					if (this->vault->registry.has<Unit>(selectedObj)) {
						this->playRandomUnitSound(selectedObj, "attack");
						this->attack(selectedObj, destEnt);
					}
					curObj++;
				}
			} else {
				this->sendGroup(this->selectedObjs, sf::Vector2i(destpos), GroupFormation::Square, North, true);
			}
		}
	}

	void handleEvent(sf::Event &event) {
		ImGuiIO& io = ImGui::GetIO();

		if (!io.WantCaptureMouse) { /* do not enable map interface if gui used */
			sf::Vector2i mousePos = sf::Mouse::getPosition(this->game->window);
			sf::Vector2f gamePos = (this->game->window.mapPixelToCoords(mousePos, this->gameView));
			sf::Vector2f gameMapPos = gamePos;
			gameMapPos.x /= 32.0;
			gameMapPos.y /= 32.0;

			switch (event.type)
			{
			case sf::Event::KeyReleased:
				this->moveView = MoveView::DontMove;
				break;
			case sf::Event::KeyPressed:
			{
				this->moveView = MoveView::DontMove;

				if (event.key.code == sf::Keyboard::Space) {
					// pause/unpause
					if (this->gameSpeed == 0) {
						this->gameSpeed = 1;
					} else {
						this->gameSpeed = 0;
					}
				}
				if (event.key.code == sf::Keyboard::Left)
					this->moveView = MoveView::MoveWest;
				if (event.key.code == sf::Keyboard::Right)
					this->moveView = MoveView::MoveEast;
				if (event.key.code == sf::Keyboard::Up)
					this->moveView = MoveView::MoveNorth;
				if (event.key.code == sf::Keyboard::Down)
					this->moveView = MoveView::MoveSouth;
				if (event.key.code == sf::Keyboard::Tab) {
					// debug window
					if (this->showDebugWindow)
						this->showDebugWindow = false;
					else
						this->showDebugWindow = true;
				}
			}
			break;
			case sf::Event::MouseMoved:
			{
				this->selectionEnd = gamePos;

				if (this->action == Action::Building)
				{
					Tile &tile = this->vault->registry.get<Tile>(this->currentBuild);
					tile.pos = sf::Vector2i(gameMapPos);
					tile.ppos = sf::Vector2f(tile.pos) * (float)32.0;
				}

				this->moveView = MoveView::DontMove;

				if (mousePos.x < 32)
					this->moveView = MoveView::MoveWest;
				if (mousePos.x > this->width - 32)
					this->moveView = MoveView::MoveEast;
				if (mousePos.y < 32)
					this->moveView = MoveView::MoveNorth;
				if (mousePos.y > this->height - 32)
					this->moveView = MoveView::MoveSouth;
			}
			break;
			case sf::Event::MouseButtonReleased:
			{
				if (event.mouseButton.button == sf::Mouse::Left) {
					if (minimap.rect.contains(sf::Vector2f(mousePos))) {
						this->clearSelection();
					} else {
						if (this->action == Action::Selecting) {
							this->selectionEnd = gamePos;
							sf::FloatRect selectRect(this->selectionStart, this->selectionEnd - this->selectionStart);
							if (selectRect.width < 0) {
								selectRect.left = selectRect.left + selectRect.width;
								selectRect.width = -selectRect.width;
							}
							if (selectRect.height < 0) {
								selectRect.top = selectRect.top + selectRect.height;
								selectRect.height = -selectRect.height;
							}

							for (int x = selectRect.left / 32.0; x < (selectRect.left + selectRect.width) / 32.0; x++) {
								for (int y = selectRect.top / 32.0; y < (selectRect.top + selectRect.height) / 32.0; y++) {
									if (this->map->bound(x, y)) {
										EntityID ent = this->map->objs.get(x, y);
										if (ent) {
											if (this->vault->registry.has<Unit>(ent)) {
												Unit &unit = this->vault->registry.get<Unit>(ent);
												GameObject &obj = this->vault->registry.get<GameObject>(ent);
												if (obj.player == this->currentPlayer) {
													this->playRandomUnitSound(obj, unit, "select");
													this->selectedObjs.push_back(ent);
												}
											}
										}
									}
								}
							}
							this->clearSelection();
						}
					}
				}
			}
			break;
			case sf::Event::MouseButtonPressed:
			{
				if (event.mouseButton.button == sf::Mouse::Left) {
					// left click on minimap
					if (this->minimap.rect.contains(sf::Vector2f(mousePos))) {
						sf::Vector2f mPos((float)(mousePos.x - this->minimap.rect.left) / (this->minimap.size / this->map->width), (float)(mousePos.y - this->minimap.rect.top) / (this->minimap.size / this->map->width));
#ifdef GAME_ENGINE_DEBUG
						std::cout << "GameEngine: minimap clicked " << mPos.x << "x" << mPos.y << std::endl;
#endif
						this->centerMapView(sf::Vector2i(mPos));
						this->clearSelection();

					} else {
						if (this->action == Action::Building)
						{
							if (this->canBuild(this->currentPlayer, this->currentBuild).size() == 0) {
								if (!this->vault->factory.placeBuilding(this->vault->registry, this->currentBuild)) {
									Player &player = this->vault->registry.get<Player>(this->currentPlayer);
									player.rootConstruction = 0;
								}

								this->action = Action::None;
								this->currentBuild = 0;
							}
						} else {
							this->action = Action::Selecting;
							this->selectionStart = gamePos;

							this->selectedObjs.clear();
							if (this->map->bound(gameMapPos.x, gameMapPos.y)) {
								EntityID entity = this->map->objs.get(gameMapPos.x, gameMapPos.y);

								// select building only
								if (entity && this->vault->registry.has<Building>(entity) && !this->vault->registry.has<Unit>(entity)) {
									GameObject &obj = this->vault->registry.get<GameObject>(entity);
									if (obj.player == this->currentPlayer)
										this->selectedObjs.push_back(entity);
								}

								this->selectedDebugObj = entity;
								if (!this->selectedDebugObj)
									this->selectedDebugObj = this->map->resources.get(gameMapPos.x, gameMapPos.y);
								if (!this->selectedDebugObj)
									this->selectedDebugObj = this->map->decors.get(gameMapPos.x, gameMapPos.y);
							}
						}
					}
				}

				if (event.mouseButton.button == sf::Mouse::Right) {
					if (this->action == Action::Building)
					{
						this->vault->registry.remove<Tile>(this->currentBuild);
						this->currentBuild = 0;
						this->action = Action::None;
						this->markUpdateLayer = true;
					} else {
						// right click on minimap
						if (this->minimap.rect.contains(sf::Vector2f(mousePos))) {
							sf::Vector2f mPos((float)(mousePos.x - this->minimap.rect.left) / (this->minimap.size / this->map->width), (float)(mousePos.y - this->minimap.rect.top) / (this->minimap.size / this->map->width));
							this->orderSelected(mPos);
						} else {
							this->orderSelected(gameMapPos);
						}
					}

					this->selectedDebugObj = 0;
				}
			}
			break;
			/* Zoom the view */
			case sf::Event::MouseWheelMoved:
			{
#ifdef ZOOMLEVEL_ENABLE
				float zoom = 2.0;

				if (event.mouseWheel.delta < 0)
				{
					this->zoomLevel *= zoom;
					if (this->zoomLevel < 8.0) {
						this->gameView.zoom(zoom);
					}
					else
						this->zoomLevel = 4.0;
				}
				else
				{
					this->zoomLevel *= 1.0 / zoom;
					if (this->zoomLevel > 0.5) {
						this->gameView.zoom(1.0 / zoom);
					}
					else
						this->zoomLevel = 1.0;
				}
#endif
			}
			break;
			}
		} else {
			this->clearSelection();
		}
	}
};

