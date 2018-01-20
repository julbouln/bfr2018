#pragma once

#include "Game.hpp"

#include "GameStage.hpp"
#include "System.hpp"
#include "Map.hpp"

#include "SimplexNoise.h"
#include "JPS.h"

#include "Components.hpp"

#include "GameSystems/GameSystem.hpp"
#include "GameSystems/TileAnimSystem.hpp"
#include "GameSystems/MapLayersSystem.hpp"
#include "GameSystems/DrawMapSystem.hpp"
#include "GameSystems/ResourcesSystem.hpp"
#include "GameSystems/PathfindingSystem.hpp"
#include "GameSystems/CombatSystem.hpp"

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
	ResourcesSystem resources;
	TileAnimSystem tileAnim;
	DrawMapSystem drawMap;
	MapLayersSystem mapLayers;
	PathfindingSystem pathfinding;
	CombatSystem combat;
	AI ai;

	sf::Sprite iface;
	sf::Sprite indice_bg;
	sf::Sprite indice;

	Action action;
	sf::Vector2f selectionStart;
	sf::Vector2f selectionEnd;

	std::vector<EntityID> selectedObjs;

	EntityID currentBuild;
	std::string currentBuildType;

	EntityID currentPlayer;

	sf::View gameView;
	sf::View guiView;

	float timePerTick;
	float currentTime;
	unsigned long ticks;
	bool markUpdateObjLayer;

	sf::Font font;

	bool scoreBonus;
	sf::Text scoreBonusText;
	sf::Sound scoreSound;

	sf::RenderTexture minimapTarget;
	sf::Sprite minimap;
	sf::FloatRect minimapRect;

	int debugCorner;
	int gameSpeed;

	MoveView moveView;

	GameEngine(Game *game) {
		this->game = game;
		this->init();
		this->generate(64, 64, "rebel");
		this->moveView = MoveView::DontMove;
	}

	GameEngine(Game *game, unsigned int mapWidth, unsigned int mapHeight, std::string playerTeam) {
		this->game = game;
		this->init();
		this->generate(mapWidth, mapHeight, playerTeam);
		this->moveView = MoveView::DontMove;
	}

	~GameEngine() {
		delete this->map;
	}

	void init() {
		this->action = Action::None;
		this->timePerTick = 0.1;
		this->ticks = 0;
		this->currentBuild = 0;
		this->markUpdateObjLayer = false;

		this->currentPlayer = 0;
		this->map = new Map();

		this->setSize(this->game->width, this->game->height);
		this->setVaults(&(this->game->vault));

		this->initView();

		this->initEffects();
		this->fadeIn();
		this->debugCorner = 1;
		this->gameSpeed = 1;

	}

	void reset() {
		this->fadeIn();
		this->nextStage = 0;
	}

	void fadeOutCallback() {
		this->game->pushRegisteredStage("play_menu");
	}

	void setVaults(GameVault *vault) {

		this->setVault(vault);
		// init systems
		tileAnim.setVault(vault);
		tileAnim.map = this->map; // not needed
		resources.setVault(vault);
		resources.map = this->map;
		drawMap.setVault(vault);
		drawMap.map = this->map;
		mapLayers.setVault(vault);
		mapLayers.map = this->map;
		pathfinding.setVault(vault);
		pathfinding.map = this->map;
		combat.setVault(vault);
		combat.map = this->map;

		ai.setVault(vault);
		ai.map = this->map;
		ai.rebelAI.setVault(this->vault);
		ai.rebelAI.map = this->map;
		ai.nazAI.setVault(this->vault);
		ai.nazAI.map = this->map;

		font.loadFromFile("medias/fonts/samos.ttf");
		scoreBonusText.setFont(font);
		scoreBonusText.setCharacterSize(48);
		scoreBonusText.setColor(sf::Color::White);

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


	void generate(unsigned int mapWidth, unsigned int mapHeight, std::string playerTeam) {
		mapLayers.initTiles();
		mapLayers.initTransitions();
		mapLayers.generate(mapWidth, mapHeight);


		if (playerTeam == "rebel") {
			this->currentPlayer = this->vault->factory.createPlayer(this->vault->registry, "rebel", true);
			this->vault->factory.createPlayer(this->vault->registry, "neonaz", true);

			this->centerMapView(sf::Vector2i(8, 8));
		} else {
			this->currentPlayer = this->vault->factory.createPlayer(this->vault->registry, "neonaz", true);
			this->vault->factory.createPlayer(this->vault->registry, "rebel", true);

			this->centerMapView(sf::Vector2i(mapWidth - 8, mapHeight - 8));
		}


		auto view = this->vault->registry.view<Player>();
		for (EntityID entity : view) {
			Player &player = view.get(entity);

			player.colorIdx = rand() % 12;

			if (player.team == "rebel")
			{

				player.initialPos = sf::Vector2i(10, 10);
				this->vault->factory.createUnit(this->vault->registry, entity, "zork", 8, 8);

				/*
								this->vault->factory.createUnit(this->vault->registry, entity, "zork", 10, 11);
								this->vault->factory.createUnit(this->vault->registry, entity, "zork", 10, 12);
								this->vault->factory.createUnit(this->vault->registry, entity, "zork", 11, 10);
								this->vault->factory.createUnit(this->vault->registry, entity, "zork", 11, 11);
								this->vault->factory.createUnit(this->vault->registry, entity, "zork", 11, 12);
								this->vault->factory.createUnit(this->vault->registry, entity, "zork", 12, 10);
								this->vault->factory.createUnit(this->vault->registry, entity, "zork", 12, 11);
								this->vault->factory.createUnit(this->vault->registry, entity, "zork", 12, 12);
								*/

				if (player.ai) {
					ai.rebelAI.parse(player.team, player.aiTree, entity);
				}

//				factory.createUnit(registry, entity, "lance_pepino", 10, 12);

			} else {
				player.initialPos = sf::Vector2i(mapWidth - 8, mapHeight - 8);
				/*
								this->vault->factory.createUnit(this->vault->registry, entity, "brad_lab", 15, 11);
								this->vault->factory.createUnit(this->vault->registry, entity, "brad_lab", 15, 12);
								this->vault->factory.createUnit(this->vault->registry, entity, "brad_lab", 16, 10);
								this->vault->factory.createUnit(this->vault->registry, entity, "brad_lab", 16, 11);
								this->vault->factory.createUnit(this->vault->registry, entity, "brad_lab", 16, 12);
								this->vault->factory.createUnit(this->vault->registry, entity, "brad_lab", 17, 10);
								this->vault->factory.createUnit(this->vault->registry, entity, "brad_lab", 17, 11);
								this->vault->factory.createUnit(this->vault->registry, entity, "brad_lab", 17, 12);
				*/

				this->vault->factory.createUnit(this->vault->registry, entity, "brad_lab", mapWidth - 10, mapHeight - 10);
//				this->vault->factory.createUnit(this->vault->registry, entity, "brad_lab", 16, 10);

				if (player.ai) {
					ai.nazAI.parse(player.team, player.aiTree, entity);
				}
			}


			player.fog.width = mapWidth;
			player.fog.height = mapHeight;
			player.fog.fill();

		}

		Player &player = this->vault->registry.get<Player>(this->currentPlayer);
		iface.setTexture(this->vault->factory.getTex("interface_" + player.team));
		indice.setTexture(this->vault->factory.getTex("indice_" + player.team));
		indice_bg.setTexture(this->vault->factory.getTex("indice_bg_" + player.team));

		minimapTarget.create(this->map->width, this->map->height);
		minimap.setTexture(minimapTarget.getTexture());

		// 128,512
		// TODO: convert to window dimension relative coord
		minimapRect = sf::FloatRect(128 - 96 / 2, 520 - 96 / 2, 96, 96);
	}

	void menuGui() {
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // Transparent background
		if (ImGui::Begin("Menu", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
		{

			if (ImGui::ImageButtonAnim(this->vault->factory.texManager.getRef("menu_button"),
			                           this->vault->factory.texManager.getRef("menu_button"),
			                           this->vault->factory.texManager.getRef("menu_button_down"))) {
				nextStage = 1;
				fadeOut();
			}
			ImGui::End();
		}
		ImGui::PopStyleColor();

	}

	void gameStateGui() {
		Player &player = this->vault->registry.get<Player>(this->currentPlayer);
		float leftDist = 192.0f;
		float topDist = 8.0f;

		ImVec2 window_pos = ImVec2(leftDist, topDist);
		ImVec2 window_pos_pivot = ImVec2(0.0f, 0.0f);
		ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // Transparent background
		if (ImGui::Begin("State", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
		{
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0, 255, 0, 255));
			ImGui::ProgressBar((float)player.resources / (float)(this->map->width * this->map->height) * 4.0, ImVec2(200.0f, 0.0f), "");
			ImGui::PopStyleColor();

			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(255, 0, 0, 255));
			ImGui::ProgressBar((float)player.butchery / 1000.0, ImVec2(200.0f, 0.0f), "");
			ImGui::PopStyleColor();

			ImGui::End();
		}
		ImGui::PopStyleColor();

	}

	void actionGui() {
		Player &player = this->vault->registry.get<Player>(this->currentPlayer);

		float rightDist = 120.0f;
		float bottomDist = 60.0f;
		ImVec2 window_pos = ImVec2(ImGui::GetIO().DisplaySize.x - rightDist, ImGui::GetIO().DisplaySize.y - bottomDist);
		ImVec2 window_pos_pivot = ImVec2(1.0f, 1.0f);
		ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // Transparent background
		if (ImGui::Begin("Actions", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
		{
			if (this->selectedObjs.size() == 1) {
				EntityID selectedObj = this->selectedObjs[0];

				Tile &tile = this->vault->registry.get<Tile>(selectedObj);
				GameObject &obj = this->vault->registry.get<GameObject>(selectedObj);
				if (this->vault->registry.has<Unit>(selectedObj)) {
					Unit &unit = this->vault->registry.get<Unit>(selectedObj);

					ImGui::BeginGroup();
					ImGui::Text("PV: %d", (int)obj.life);
					ImGui::Text("AC: %d", unit.attack1.power);
					ImGui::Text("DC: %d", unit.attack1.distance);
					ImGui::Text("AE: %d", unit.attack2.power);
					ImGui::Text("DE: %d", unit.attack2.distance);
					ImGui::EndGroup();

					ImGui::SameLine();

					ImGui::BeginGroup();
					if (ImGui::ImageButtonAnim(this->vault->factory.texManager.getRef(player.team + "_move"),
					                           this->vault->factory.texManager.getRef(player.team + "_move"),
					                           this->vault->factory.texManager.getRef(player.team + "_move_down"))) {
						std::cout << "move clicked " << std::endl;

					}

					ImGui::SameLine();
					if (ImGui::ImageButtonAnim(this->vault->factory.texManager.getRef(player.team + "_attack"),
					                           this->vault->factory.texManager.getRef(player.team + "_attack"),
					                           this->vault->factory.texManager.getRef(player.team + "_attack_down"))) {
						std::cout << "attack clicked " << std::endl;

					}
					ImGui::EndGroup();

				}

				TechNode *pnode = this->vault->factory.getTechNode(player.team, obj.name);
				if (pnode->children.size() > 0) {
					for (TechNode &node : pnode->children) {
						if (ImGui::ImageButtonAnim(this->vault->factory.texManager.getRef(node.type + "_icon"),
						                           this->vault->factory.texManager.getRef(node.type + "_icon"),
						                           this->vault->factory.texManager.getRef(node.type + "_icon_down"))) {
							std::cout << "build clicked " << node.type << std::endl;
							switch (node.comp) {
							case TechComponent::Building:
								this->action = Action::Building;
								this->currentBuildType = node.type;
								break;
							case TechComponent::Character:
								if (this->trainUnit(node.type, this->currentPlayer, selectedObj)) {
									this->markUpdateObjLayer = true;
								}
								break;
							case TechComponent::Resource:
								this->seedResources(player.resourceType, selectedObj);
								break;
							}
						}
						ImGui::SameLine();
					}
				}


			} else {
				if (this->selectedObjs.size() == 0) {
					TechNode *node = this->vault->factory.getTechRoot(player.team);
					if (ImGui::ImageButtonAnim(this->vault->factory.texManager.getRef(node->type + "_icon"),
					                           this->vault->factory.texManager.getRef(node->type + "_icon"),
					                           this->vault->factory.texManager.getRef(node->type + "_icon_down"))) {
						std::cout << "build clicked " << node->type << std::endl;
						this->action = Action::Building;
						this->currentBuildType = node->type;
					}
				}
			}
			ImGui::End();

		}
		ImGui::PopStyleColor();
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

			ImGui::Text("Speed"); ImGui::SameLine();
			ImGui::RadioButton("0", &gameSpeed, 0); ImGui::SameLine();
			ImGui::RadioButton("x1", &gameSpeed, 1); ImGui::SameLine();
			ImGui::RadioButton("x4", &gameSpeed, 4); ImGui::SameLine();
			ImGui::RadioButton("x16", &gameSpeed, 16);

			this->setGameSpeed(gameSpeed);

			if (this->selectedObjs.size() > 0)
			{
				if (this->selectedObjs.size() > 1) {
					ImGui::Text("Selected entities: %d", (int)this->selectedObjs.size());
				} else {
					EntityID selectedObj = this->selectedObjs.front();
					if (this->vault->registry.valid(selectedObj)) {
						GameObject &obj = this->vault->registry.get<GameObject>(selectedObj);
						Tile &tile = this->vault->registry.get<Tile>(selectedObj);
						ImGui::Text("Name: %s", obj.name.c_str());
						ImGui::Text("Team: %s", obj.team.c_str());
						ImGui::Text("Case position: %dx%d", tile.pos.x, tile.pos.y);
						ImGui::Text("Case size: %dx%d", tile.size.x, tile.size.y);
						ImGui::Text("Pixel position: %.2fx%.2f", tile.ppos.x, tile.ppos.y);
						ImGui::Text("Pixel size: %.2fx%.2f", tile.psize.x, tile.psize.y);
						ImGui::Text("Direction: %d", tile.direction);
						ImGui::Text("State: %s", tile.state.c_str());
						ImGui::Text("Current frame: %d", tile.animHandlers[tile.state].getCurrentFrame());
						ImGui::Text("Current anim: %d", tile.animHandlers[tile.state].getCurrentAnim());
						ImGui::Text("Life: %f", obj.life);
						if (this->vault->registry.has<Unit>(selectedObj)) {
							Unit &unit = this->vault->registry.get<Unit>(selectedObj);
							ImGui::Text("Next pos: %dx%d", unit.nextpos.x, unit.nextpos.y);
							ImGui::Text("Dest pos: %dx%d", unit.destpos.x, unit.destpos.y);
							ImGui::Text("Dest attack: %d", (int)unit.destAttack);
						}
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

		auto objView = this->vault->registry.view<GameObject>();
		for (EntityID entity : objView) {
			GameObject &obj = objView.get(entity);

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
			for (auto o : player.objsByType) {
				playerObjs += o.second.size();
			}
			std::cout << "Player: " << entity << " " << player.team << " objs:" << playerObjs << " resources:" << player.resources << " butchery:" << player.butchery << std::endl;
		}

	}

	void updateDecade(float dt) {
		auto playerView = this->vault->registry.view<Player>();

		for (EntityID entity : playerView) {
			Player &player = playerView.get(entity);
//			std::cout << "Player "<<entity<< " "<<player.team<<" kills "<<player.kills.size() << std::endl;

			player.butchery += pow(player.kills.size(), 2);

			if (entity == this->currentPlayer) {
				switch (player.kills.size()) {
				case 0:
					this->scoreBonus = false;
					break;
				case 1:
					this->scoreBonus = false;
					// normal
					break;
				case 2:
					std::cout << "! COMBO " << player.team <<  std::endl;
					this->scoreBonus = true;
					this->scoreBonusText.setString("COMBO");
					this->playSound(scoreSound, "combo");
					break;
				case 3:
					std::cout << "! SERIAL-KILLER " << player.team <<  std::endl;
					this->scoreBonus = true;
					this->scoreBonusText.setString("SERIAL KILLER");
					this->playSound(scoreSound, "killer");
					break;
				case 4:
					std::cout << "! MEGAKILL " << player.team << std::endl;
					this->scoreBonus = true;
					this->scoreBonusText.setString("MEGAKILL");
					this->playSound(scoreSound, "megakill");
					break;
				case 5:
					std::cout << "! BARBARIAN " << player.team << std::endl;
					this->scoreBonus = true;
					this->scoreBonusText.setString("BARBARIAN");
					this->playSound(scoreSound, "barbarian");
					break;
				default: // >= 6
					std::cout << "! BUTCHERY " << player.team << std::endl;
					this->scoreBonus = true;
					this->scoreBonusText.setString("BUTCHERY");
					this->playSound(scoreSound, "butchery");
					break;

				}
			}
			player.kills.clear();
		}

	}

	void updateEveryFrame(float dt)
	{
		if (this->markUpdateObjLayer) {
			this->mapLayers.updateObjsLayer(0);
			this->markUpdateObjLayer = false;
		}

		this->tileAnim.update(dt);
		this->pathfinding.update(dt);
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
		drawMap.draw(this->game->window, clip, dt);

		// draw selected
		for (EntityID selectedObj : this->selectedObjs) {
			//if (this->vault->registry.valid(selectedObj))
			{
				Tile &tile = this->vault->registry.get<Tile>(selectedObj);
				sf::RectangleShape rectangle;

				sf::Vector2f pos;
				pos.x = tile.ppos.x - (tile.centerRect.left + tile.centerRect.width / 2) + 16 + tile.offset.x * 32;
				pos.y = tile.ppos.y - (tile.centerRect.top + tile.centerRect.height / 2) + 16 + tile.offset.y * 32;


				rectangle.setSize(sf::Vector2f(tile.psize));
				rectangle.setFillColor(sf::Color(0x00, 0x00, 0x00, 0x00));
				rectangle.setOutlineColor(sf::Color::Blue);
				rectangle.setOutlineThickness(2);
				rectangle.setPosition(pos);

				this->game->window.draw(rectangle);
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


		this->game->window.setView(this->guiView);

		iface.setPosition(sf::Vector2f(0, 0));
		iface.setScale(this->width / 800.0, this->height / 600.0);
		this->game->window.draw(iface);
		this->debugGui(dt);
		this->menuGui();
		this->gameStateGui();
		this->actionGui();

		if (this->scoreBonus) {
			sf::FloatRect textRect = this->scoreBonusText.getLocalBounds();
			scoreBonusText.setOrigin(textRect.left + textRect.width / 2.0f,
			                         textRect.top  + textRect.height / 2.0f);
			scoreBonusText.setPosition(sf::Vector2f(this->width / 2, this->height / 2));

			this->game->window.draw(scoreBonusText);
		}


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

		drawMap.drawMinimap(this->game->window, minimapTarget, clip, this->currentPlayer);
		minimap.setPosition(sf::Vector2f(minimapRect.left, minimapRect.top));
		minimap.setScale(sf::Vector2f(96.0 / this->map->width, 96.0 / this->map->height));
		this->game->window.draw(minimap);

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

	void update(float dt) {
//		std::cout << "GameEngine: update " << dt << std::endl;
		this->game->window.setView(this->gameView);

		if (this->timePerTick == FLT_MAX) return;

		this->updateEveryFrame(dt);

		this->currentTime += dt;
		if (this->currentTime < this->timePerTick) return;

		this->ticks++;
		this->currentTime = 0.0;

		if (this->ticks % 100 == 0) {
			this->updateHundred(dt);
		}

		if (this->ticks % 10 == 0) {
			this->updateDecade(dt);
		}

		if (this->action == Action::Building && this->currentBuild == 0) {
			this->currentBuild = this->vault->factory.createBuilding(this->vault->registry, this->currentPlayer, this->currentBuildType, 8, 8, false);
		}

		this->updatePlayers(dt);

		this->combat.update(dt);
		this->resources.update(dt);
		this->mapLayers.update(dt);

		this->mapLayers.updateFog(this->currentPlayer, dt);

		// AI
		auto playerView = this->vault->registry.view<Player>();
		for (EntityID entity : playerView) {
			Player &player = playerView.get(entity);
			if (player.ai)
				player.aiTree.update();
		}

		this->updateSelected(dt);

		switch(this->moveView) {
			case MoveView::DontMove:
			break;
			case MoveView::MoveWest:
				this->gameView.move(sf::Vector2f{ -16.0, 0.0});
			break;
			case MoveView::MoveEast:
				this->gameView.move(sf::Vector2f{16.0, 0.0});
			break;
			case MoveView::MoveNorth:
				this->gameView.move(sf::Vector2f{ 0.0, -16.0});
			break;
			case MoveView::MoveSouth:
				this->gameView.move(sf::Vector2f{0.0, 16.0});
			break;
		}
	}

	void handleEvent(sf::Event &event) {
		sf::Vector2i mousePos = sf::Mouse::getPosition(this->game->window);
		sf::Vector2f gamePos = (this->game->window.mapPixelToCoords(mousePos, this->gameView));
		sf::Vector2f gameMapPos = gamePos;
		gameMapPos.x /= 32.0;
		gameMapPos.y /= 32.0;

		switch (event.type)
		{

		case sf::Event::KeyPressed:
		{
			if (event.key.code == sf::Keyboard::Left)
				this->gameView.move(sf::Vector2f{ -16.0, 0.0});
			if (event.key.code == sf::Keyboard::Right)
				this->gameView.move(sf::Vector2f{16.0, 0.0});
			if (event.key.code == sf::Keyboard::Up)
				this->gameView.move(sf::Vector2f{0.0, -16.0});
			if (event.key.code == sf::Keyboard::Down)
				this->gameView.move(sf::Vector2f{0.0, 16.0});
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
				if (this->minimapRect.contains(sf::Vector2f(mousePos))) {
				} else {

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

					std::cout << "END SELECTION " << selectRect.left << "x" << selectRect.top << ":" << selectRect.width << "x" << selectRect.height << std::endl;
					this->selectionStart = sf::Vector2f(0, 0);
					this->selectionEnd = sf::Vector2f(0, 0);
					this->action = Action::None;
				}
			}
		}
		break;
		case sf::Event::MouseButtonPressed:
		{
			if (event.mouseButton.button == sf::Mouse::Left) {
				if (this->minimapRect.contains(sf::Vector2f(mousePos))) {
//					sf::IntRect mRect = sf::IntRect(this->minimapRect);
					sf::Vector2f mPos((float)(mousePos.x - this->minimapRect.left) / (96.0 / this->map->width), (float)(mousePos.y - this->minimapRect.top) / (96.0 / this->map->width));

					std::cout << "minimap clicked " << mPos.x << "x" << mPos.y << std::endl;
					this->centerMapView(sf::Vector2i(mPos));
				} else {

					if (this->action == Action::Building)
					{
						Building &building = this->vault->registry.get<Building>(this->currentBuild);
						GameObject &obj = this->vault->registry.get<GameObject>(this->currentBuild);
						obj.mapped = true;
						this->action = Action::None;
						this->currentBuild = 0;
					} else {
						this->action = Action::Selecting;
						std::cout << "START SELECTION" << std::endl;
						this->selectionStart = gamePos;

						this->selectedObjs.clear();
						EntityID entity = this->map->objs.get(gameMapPos.x, gameMapPos.y);

						if (entity && this->vault->registry.has<Building>(entity)) {
							GameObject &obj = this->vault->registry.get<GameObject>(entity);
							if (obj.player == this->currentPlayer)
								this->selectedObjs.push_back(entity);
						}
					}
				}
			}

			if (event.mouseButton.button == sf::Mouse::Right) {
				if (this->action == Action::Building)
				{
					this->vault->registry.destroy(this->currentBuild);
					this->currentBuild = 0;
					this->action = Action::None;
					this->markUpdateObjLayer = true;

				} else {
					if (this->selectedObjs.size() > 0) {
						double squareD = sqrt((double)this->selectedObjs.size());
						int square = ceil(squareD);
						int curObj = 0;

						EntityID destEnt = this->ennemyAtPosition(this->currentPlayer, gameMapPos.x, gameMapPos.y);

						if (destEnt) {
							while (curObj < this->selectedObjs.size()) {
								EntityID selectedObj = this->selectedObjs[curObj];
								if (this->vault->registry.has<Unit>(selectedObj)) {
									this->playRandomUnitSound(selectedObj, "attack");
									this->attack(selectedObj, destEnt);
								}
								curObj++;
							}

						} else {

							for (int x = 0; x < square; x++) {
								for (int y = 0; y < square; y++) {
									if (curObj < this->selectedObjs.size()) {
										EntityID selectedObj = this->selectedObjs[curObj];
										if (this->vault->registry.has<Unit>(selectedObj)) {
											this->goTo(selectedObj, sf::Vector2i(gameMapPos.x + x, gameMapPos.y + y));
											this->playRandomUnitSound(selectedObj, "move");

											Unit &unit = this->vault->registry.get<Unit>(selectedObj);
											unit.destAttack = 0;
										}
										curObj++;
									}
								}
							}
						}
					}
				}
			}
		}
		break;

		}
	}
};

