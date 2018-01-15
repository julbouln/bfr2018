#pragma once

#include "GameVault.hpp"
#include "System.hpp"
#include "Map.hpp"

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

class GameEngine : public GameSystem {
public:
	ResourcesSystem resources;
	TileAnimSystem tileAnim;
	DrawMapSystem drawMap;
	MapLayersSystem mapLayers;
	PathfindingSystem pathfinding;
	CombatSystem combat;

	sf::Sprite iface;

	Action action;
	sf::Vector2f selectionStart;
	sf::Vector2f selectionEnd;

	std::vector<EntityID> selectedObjs;

	EntityID currentBuild;
	std::string currentBuildType;

	bool markUpdateObjLayer;

	EntityID currentPlayer;

	sf::View gameView;
	sf::View guiView;

	float timePerTick;
	float currentTime;

	unsigned int width;
	unsigned int height;

	GameEngine(GameVault *vault) {
		this->setVault(vault);
		this->action = Action::None;
		this->timePerTick = 0.1;
		this->currentBuild = 0;
		this->markUpdateObjLayer = false;

		this->currentPlayer = 0;
		this->map = new Map();

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

	}

	void setSize(unsigned int width, unsigned int height) {
		this->width = width;
		this->height = height;
	}

	void initView(sf::RenderWindow &window) {
		sf::Vector2f pos = sf::Vector2f(window.getSize());
		guiView.setSize(pos);
		gameView.setSize(pos);
		pos *= 0.5f;
		guiView.setCenter(pos);
		gameView.setCenter(pos);
	}

	void generate(unsigned int mapWidth, unsigned int mapHeight) {
		map->initTiles(this->vault->registry, this->vault->factory);
		map->generate(this->vault->registry, this->vault->factory, mapWidth, mapHeight);

		this->currentPlayer = this->vault->factory.createPlayer(this->vault->registry, "rebel", false);
		this->vault->factory.createPlayer(this->vault->registry, "neonaz", true);

		auto view = this->vault->registry.view<Player>();
		for (EntityID entity : view) {
			Player &player = view.get(entity);
			if (player.team == "rebel")
			{
				this->vault->factory.createUnit(this->vault->registry, entity, "zork", 10, 10);
//				factory.createUnit(registry, entity, "lance_pepino", 10, 12);
			} else {
				this->vault->factory.createUnit(this->vault->registry, entity, "brad_lab", mapWidth - 10, mapHeight - 10);
			}
		}

		Player &player = this->vault->registry.get<Player>(this->currentPlayer);
		iface.setTexture(this->vault->factory.getTex("interface_" + player.team));

//		factory.createUnit(registry, this->currentPlayer, "zork", 10, 10);
		/*
				factory.createUnit(registry, this->currentPlayer, "zork", 10, 11);
				factory.createUnit(registry, this->currentPlayer, "zork", 10, 12);
				factory.createUnit(registry, this->currentPlayer, "zork", 11, 10);
				factory.createUnit(registry, this->currentPlayer, "zork", 11, 11);
				factory.createUnit(registry, this->currentPlayer, "zork", 11, 12);
				factory.createUnit(registry, this->currentPlayer, "zork", 12, 10);
				factory.createUnit(registry, this->currentPlayer, "zork", 12, 11);
				factory.createUnit(registry, this->currentPlayer, "zork", 12, 12);
				*/
		/*
						factory.createBuilding(registry, this->currentPlayer, "taverne", 16, 10, true);
				*/
//		map.objs.set(10, 10, factory.createUnit(registry, this->currentPlayer, "zork", 10, 10));
	}


	void handleEvent(sf::RenderWindow &window, sf::Event &event) {
		sf::Vector2f gamePos = (window.mapPixelToCoords(sf::Mouse::getPosition(window), this->gameView));
		sf::Vector2f gameMapPos = gamePos;
		gameMapPos.x /= 32.0;
		gameMapPos.y /= 32.0;

		switch (event.type)
		{

		case sf::Event::KeyPressed:
		{
			if (event.key.code == sf::Keyboard::Left)
				gameView.move(sf::Vector2f{ -16.0, 0.0});
			if (event.key.code == sf::Keyboard::Right)
				gameView.move(sf::Vector2f{16.0, 0.0});
			if (event.key.code == sf::Keyboard::Up)
				gameView.move(sf::Vector2f{0.0, -16.0});
			if (event.key.code == sf::Keyboard::Down)
				gameView.move(sf::Vector2f{0.0, 16.0});
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
		}
		break;
		case sf::Event::MouseButtonReleased:
		{
			if (event.mouseButton.button == sf::Mouse::Left) {
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
									GameObject &obj = this->vault->registry.get<GameObject>(ent);
									if (obj.player == this->currentPlayer)
										this->selectedObjs.push_back(ent);
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
		break;
		case sf::Event::MouseButtonPressed:
		{
			if (event.mouseButton.button == sf::Mouse::Left) {
				if (this->action == Action::Building)
				{
					Building &building = this->vault->registry.get<Building>(this->currentBuild);
					building.built = true;
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
									std::cout << "SET ATTACK " << selectedObj << std::endl;
									Unit &unit = this->vault->registry.get<Unit>(selectedObj);
									unit.destAttack = destEnt;

								}
								curObj++;
							}

							std::cout << "ATTACK " << destEnt << std::endl;
						} else {

							for (int x = 0; x < square; x++) {
								for (int y = 0; y < square; y++) {
									if (curObj < this->selectedObjs.size()) {
										EntityID selectedObj = this->selectedObjs[curObj];
										if (this->vault->registry.has<Unit>(selectedObj)) {
											Tile &tile = this->vault->registry.get<Tile>(selectedObj);
											Unit &unit = this->vault->registry.get<Unit>(selectedObj);
											unit.nextpos = tile.pos;
											sf::Vector2i dpos;
											dpos.x = gameMapPos.x + x;
											dpos.y = gameMapPos.y + y;
											unit.destpos = dpos;
											unit.nopath = 0;
											tile.ppos = sf::Vector2f(tile.pos) * (float)32.0;

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

	void draw(sf::RenderWindow &window, float dt) {
		drawMap.draw(window, dt);

		// draw selected
		for (EntityID selectedObj : this->selectedObjs) {
			Tile &tile = this->vault->registry.get<Tile>(selectedObj);
			sf::RectangleShape rectangle;

			sf::Vector2f pos;
//			pos.x = tile.ppos.x;
//			pos.y = tile.ppos.y;

			pos.x = tile.ppos.x - tile.psize.x / 2 + 16;
			pos.y = tile.ppos.y - tile.psize.y / 2;


			rectangle.setSize(sf::Vector2f(tile.psize));
			rectangle.setFillColor(sf::Color(0x00, 0x00, 0x00, 0x00));
			rectangle.setOutlineColor(sf::Color::Blue);
			rectangle.setOutlineThickness(2);
			rectangle.setPosition(pos);

			window.draw(rectangle);
		}

		// draw debug grid
		for (int y = 0; y < this->map->height; ++y)
		{
			for (int x = 0; x < this->map->width; ++x)
			{
				if (this->map->objs.get(x, y)) {
					sf::RectangleShape rectangle;

					sf::Vector2f pos;
					pos.x = x * 32;
					pos.y = y * 32;

					rectangle.setSize(sf::Vector2f(32, 32));
					rectangle.setFillColor(sf::Color(0x00, 0x00, 0x00, 0x00));
					rectangle.setOutlineColor(sf::Color(0xff, 0xff, 0xff, 0x7f));
					rectangle.setOutlineThickness(1);
					rectangle.setPosition(pos);

					window.draw(rectangle);

				}

				if (this->map->resources.get(x, y)) {
					sf::RectangleShape rectangle;

					sf::Vector2f pos;
					pos.x = x * 32;
					pos.y = y * 32;

					rectangle.setSize(sf::Vector2f(32, 32));
					rectangle.setFillColor(sf::Color(0x00, 0x00, 0x00, 0x00));
					rectangle.setOutlineColor(sf::Color(0x00, 0xff, 0x00, 0x7f));
					rectangle.setOutlineThickness(1);
					rectangle.setPosition(pos);

					window.draw(rectangle);

				}
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

			window.draw(rectangle);
		}


		window.setView(this->guiView);

		iface.setPosition(sf::Vector2f(0, 0));
		iface.setScale(this->width / 800.0, this->height / 600.0);
		window.draw(iface);
		this->testGui();
		this->gameStateGui();
		this->actionGui();
	}

	void gameStateGui() {
		Player &player = this->vault->registry.get<Player>(this->currentPlayer);
		float leftDist = 200.0f;
		float topDist = 8.0f;

		ImVec2 window_pos = ImVec2(leftDist, topDist);
		ImVec2 window_pos_pivot = ImVec2(0.0f, 0.0f);
		ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // Transparent background
		if (ImGui::Begin("State", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
		{
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0, 255, 0, 255));
			ImGui::ProgressBar((float)player.resources / (float)(this->map->width * this->map->height), ImVec2(200.0f, 0.0f), "");
			ImGui::PopStyleColor();

			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(255, 0, 0, 255));
			ImGui::ProgressBar(player.butchery, ImVec2(200.0f, 0.0f), "");
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
//			std::cout << "ACTIONS "<<this->selectedObjs.size()<<std::endl;
			if (this->selectedObjs.size() == 1) {
				EntityID selectedObj = this->selectedObjs[0];

				Tile &tile = this->vault->registry.get<Tile>(selectedObj);
				GameObject &obj = this->vault->registry.get<GameObject>(selectedObj);
				if (this->vault->registry.has<Unit>(selectedObj)) {
					Unit &unit = this->vault->registry.get<Unit>(selectedObj);

					ImGui::BeginGroup();
					ImGui::Text("PV: %d", obj.life);
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

	void testGui() {
		const float DISTANCE = 10.0f;
		static int corner = 0;
		ImVec2 window_pos = ImVec2((corner & 1) ? ImGui::GetIO().DisplaySize.x - DISTANCE : DISTANCE, (corner & 2) ? ImGui::GetIO().DisplaySize.y - DISTANCE : DISTANCE);
		ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
		ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.3f)); // Transparent background
		ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[2]);
		if (ImGui::Begin("Example: Fixed Overlay", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
		{
			ImGui::Text("Entities: %d", this->vault->registry.size());
			ImGui::Text("Simple overlay\nin the corner of the screen.\n(right-click to change position)");
			ImGui::Separator();
			ImGui::Text("Mouse Position: (%.1f,%.1f)", ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y);

			if (ImGui::BeginPopupContextWindow())
			{
				if (ImGui::MenuItem("Top-left", NULL, corner == 0)) corner = 0;
				if (ImGui::MenuItem("Top-right", NULL, corner == 1)) corner = 1;
				if (ImGui::MenuItem("Bottom-left", NULL, corner == 2)) corner = 2;
				if (ImGui::MenuItem("Bottom-right", NULL, corner == 3)) corner = 3;
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
			player.objsCount.clear();
		}

		auto objView = this->vault->registry.view<GameObject>();
		for (EntityID entity : objView) {
			GameObject &obj = objView.get(entity);

			if (obj.player)
			{
				Player &player = this->vault->registry.get<Player>(obj.player);
				if (player.objsCount.count(obj.name)) {
					player.objsCount[obj.name] = player.objsCount[obj.name] + 1;
				} else {
					player.objsCount[obj.name] = 0;
				}
			}
		}
	}

	void update(float dt) {
		this->updateEveryFrame(dt);
		this->currentTime += dt;
		if (this->currentTime < this->timePerTick) return;

		this->currentTime = 0.0;

		if (this->action == Action::Building && this->currentBuild == 0) {
			this->currentBuild = this->vault->factory.createBuilding(this->vault->registry, this->currentPlayer, this->currentBuildType, 8, 8, false);
			std::cout << "built " << this->currentBuild << std::endl;
		}

		this->updatePlayers(dt);

		this->combat.update(dt);
		this->resources.update(dt);
		this->mapLayers.update(dt);
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

};

