#pragma once

#include "Map.hpp"

enum class Action {
	None,
	Selecting,
	Building
};

class GameEngine {
public:
	sf::Sprite iface;

	Action action;
	sf::Vector2f selectionStart;
	sf::Vector2f selectionEnd;

	std::vector<EntityID> selectedObjs;

	EntityID currentBuild;
	std::string currentBuildType;

	bool markUpdateObjLayer;

	EntityID currentPlayer;

	Map map;

	sf::View gameView;
	sf::View guiView;

	float timePerTick;
	float currentTime;

	unsigned int width;
	unsigned int height;

	GameEngine() {
		this->action = Action::None;
		this->timePerTick = 0.1;
		this->currentBuild = 0;
		this->markUpdateObjLayer = false;

		this->currentPlayer = 0;
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

	void generate(entt::Registry<EntityID> &registry, EntityFactory &factory) {
		unsigned int mapWidth = 64;
		unsigned int mapHeight = 64;
		map.initTiles(registry, factory);
		map.generate(registry, factory, mapWidth, mapHeight);

		this->currentPlayer = factory.createPlayer(registry, "rebel", false);
		factory.createPlayer(registry, "neonaz", true);

		auto view = registry.view<Player>();
		for (EntityID entity : view) {
			Player &player = view.get(entity);
			if (player.team == "rebel")
			{
//				factory.createUnit(registry, entity, "zork", 10, 10);
//				factory.createUnit(registry, entity, "lance_pepino", 10, 12);
			} else {
				factory.createUnit(registry, entity, "brad_lab", 15, 10);
			}
		}

		Player &player = registry.get<Player>(this->currentPlayer);
		iface.setTexture(factory.getTex("interface_" + player.team));

//		factory.createUnit(registry, this->currentPlayer, "zork", 10, 10);

		factory.createUnit(registry, this->currentPlayer, "zork", 10, 11);
		factory.createUnit(registry, this->currentPlayer, "zork", 10, 12);
		factory.createUnit(registry, this->currentPlayer, "zork", 11, 10);
		factory.createUnit(registry, this->currentPlayer, "zork", 11, 11);
		factory.createUnit(registry, this->currentPlayer, "zork", 11, 12);
		factory.createUnit(registry, this->currentPlayer, "zork", 12, 10);
		factory.createUnit(registry, this->currentPlayer, "zork", 12, 11);
		factory.createUnit(registry, this->currentPlayer, "zork", 12, 12);
		/*
						factory.createBuilding(registry, this->currentPlayer, "taverne", 16, 10, true);
				*/
//		map.objs.set(10, 10, factory.createUnit(registry, this->currentPlayer, "zork", 10, 10));
	}


	void handleEvent(entt::Registry<EntityID> &registry, sf::RenderWindow &window, sf::Event &event) {
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
				Tile &tile = registry.get<Tile>(this->currentBuild);
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
						if (x > 0 && y > 0 && x < map.width && y < map.height) {
							EntityID ent = map.objs.get(x, y);
							if (ent) {
								if (registry.has<Unit>(ent)) {
									GameObject &obj = registry.get<GameObject>(ent);
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
					Building &building = registry.get<Building>(this->currentBuild);
					building.built = true;
					this->action = Action::None;
					this->currentBuild = 0;
				} else {
					this->action = Action::Selecting;
					std::cout << "START SELECTION" << std::endl;
					this->selectionStart = gamePos;

					this->selectedObjs.clear();
					EntityID entity = map.objs.get(gameMapPos.x, gameMapPos.y);

					if (entity && registry.has<Building>(entity)) {
						GameObject &obj = registry.get<GameObject>(entity);
						if (obj.player == this->currentPlayer)
							this->selectedObjs.push_back(entity);
					}
				}
			}

			if (event.mouseButton.button == sf::Mouse::Right) {
				if (this->action == Action::Building)
				{
					registry.destroy(this->currentBuild);
					this->currentBuild = 0;
					this->action = Action::None;
					this->markUpdateObjLayer = true;

				} else {
					if (this->selectedObjs.size() > 0) {
						double squareD = sqrt((double)this->selectedObjs.size());
						int square = ceil(squareD);
						int curObj = 0;

						EntityID destEnt = this->ennemyAtPosition(registry, this->currentPlayer, gameMapPos.x, gameMapPos.y);

						if (destEnt) {
							while (curObj < this->selectedObjs.size()) {
								EntityID selectedObj = this->selectedObjs[curObj];
								if (registry.has<Unit>(selectedObj)) {
									std::cout << "SET ATTACK " << selectedObj << std::endl;
									Unit &unit = registry.get<Unit>(selectedObj);
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
										if (registry.has<Unit>(selectedObj)) {
											Tile &tile = registry.get<Tile>(selectedObj);
											Unit &unit = registry.get<Unit>(selectedObj);
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

	EntityID ennemyAtPosition(entt::Registry<EntityID> &registry, EntityID playerEnt, int x, int y) {
		Player &player = registry.get<Player>(playerEnt);
		EntityID destEnt = map.objs.get(x, y);
		if (destEnt) {
			GameObject &obj = registry.get<GameObject>(destEnt);
			if (obj.team != player.team)
				return destEnt;
		}
		return 0;
	}

	void drawTileLayer(entt::Registry<EntityID> &registry, EntityFactory &factory, TileLayer &layer, sf::RenderWindow &window, float dt) {
		sf::View wview = window.getView();
		sf::FloatRect screenRect(sf::Vector2f(wview.getCenter().x - (wview.getSize().x) / 2, wview.getCenter().y - (wview.getSize().y) / 2) , wview.getSize());

		int mx = screenRect.left / 32.0;
		int my = screenRect.top / 32.0;
		int mw = mx + screenRect.width / 32.0;
		int mh = my + screenRect.height / 32.0;

		mx = mx < 0 ? 0 : mx;
		my = my < 0 ? 0 : my;
		mw = mw > layer.width ? layer.width : mw;
		mh = mh > layer.height ? layer.height : mh;

		for (int y = my; y < mh; ++y)
		{
			for (int x = mx; x < mw; ++x)
			{
				EntityID ent = layer.get(x, y);
				if (ent) {
					Tile &tile = registry.get<Tile>(ent);

					sf::Vector2f pos;
					pos.x = tile.ppos.x;
					pos.y = tile.ppos.y;
					pos.x = x * 32;
					pos.y = y * 32;

					tile.sprite.setPosition(pos);


					/* Draw the tile */
					window.draw(tile.sprite);
				}
			}

		}
	}

	void drawObjLayer(entt::Registry<EntityID> &registry, EntityFactory &factory, sf::RenderWindow &window, float dt) {
		sf::View wview = window.getView();
		sf::FloatRect screenRect(sf::Vector2f(wview.getCenter().x - (wview.getSize().x) / 2, wview.getCenter().y - (wview.getSize().y) / 2) , wview.getSize());

		for (EntityID ent : map.entities) {
			Tile &tile = registry.get<Tile>(ent);

			sf::Vector2f pos;

//		pos.x = tile.ppos.x - tile.offset.x * 32;
//		pos.y = tile.ppos.y - tile.offset.y * 32;

			pos.x = tile.ppos.x - tile.psize.x / 2 + 16;
			pos.y = tile.ppos.y - tile.psize.y / 2;

			tile.sprite.setPosition(pos);

			sf::FloatRect collider(tile.sprite.getGlobalBounds().left,
			                       tile.sprite.getGlobalBounds().top, 32, 32);

			/* Draw the tile */
			if (screenRect.intersects(collider))
				window.draw(tile.sprite);
		}
	}


	void draw(entt::Registry<EntityID> &registry, EntityFactory &factory, sf::RenderWindow &window, float dt) {
		if (this->markUpdateObjLayer) {
			this->updateObjsLayer(registry, factory, 0);
			this->markUpdateObjLayer = false;
		}

		this->drawTileLayer(registry, factory, map.terrains, window, dt);
		this->drawObjLayer(registry, factory, window, dt);

		for (EntityID selectedObj : this->selectedObjs) {
			Tile &tile = registry.get<Tile>(selectedObj);
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

//			rectangle.setOrigin(sf::Vector2f(sf::Vector2f(tile.psize.x / 2, tile.psize.y / 2)));

			window.draw(rectangle);
		}

		for (int y = 0; y < map.height; ++y)
		{
			for (int x = 0; x < map.width; ++x)
			{
				if (map.objs.get(x, y)) {
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

				if (map.resources.get(x, y)) {
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
		this->testGui(registry, factory);
		this->gameStateGui(registry, factory);
		this->actionGui(registry, factory);
	}


	bool spendResources(entt::Registry<EntityID> &registry, EntityFactory &factory, EntityID playerEnt, ResourceType type, int val) {
		Player &player = registry.get<Player>(playerEnt);
		if (player.resources > val) {
			auto view = registry.view<Resource>();
			for (EntityID entity : view) {
				Resource &resource = view.get(entity);
				if (resource.type == type) {
					val -= resource.level;
					registry.destroy(entity);
					if (val <= 0)
						break;
				}
			}
			this->updateObjsLayer(registry, factory, 0.0);
			return true;
		} else {
			return false;
		}
	}

	void seedResources(entt::Registry<EntityID> &registry, EntityFactory &factory, ResourceType type, EntityID entity) {
		Tile &tile = registry.get<Tile>(entity);
		for (sf::Vector2i p : this->tileAround(tile, 1)) {
			float rnd = ((float) rand()) / (float) RAND_MAX;
			if (rnd > 0.8) {
				if (!map.resources.get(p.x, p.y) && !map.objs.get(p.x, p.y))
					factory.plantResource(registry, type, p.x, p.y);
			}
		}
	}

	void gameStateGui(entt::Registry<EntityID> &registry, EntityFactory &factory) {
		Player &player = registry.get<Player>(this->currentPlayer);
		float leftDist = 200.0f;
		float topDist = 8.0f;

		ImVec2 window_pos = ImVec2(leftDist, topDist);
		ImVec2 window_pos_pivot = ImVec2(0.0f, 0.0f);
		ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // Transparent background
		if (ImGui::Begin("State", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
		{
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0, 255, 0, 255));
			ImGui::ProgressBar((float)player.resources / (float)(map.width * map.height), ImVec2(200.0f, 0.0f), "");
			ImGui::PopStyleColor();

			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(255, 0, 0, 255));
			ImGui::ProgressBar(player.butchery, ImVec2(200.0f, 0.0f), "");
			ImGui::PopStyleColor();

			ImGui::End();
		}
		ImGui::PopStyleColor();

	}

	void actionGui(entt::Registry<EntityID> &registry, EntityFactory &factory) {
		Player &player = registry.get<Player>(this->currentPlayer);

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

				Tile &tile = registry.get<Tile>(selectedObj);
				GameObject &obj = registry.get<GameObject>(selectedObj);
				if (registry.has<Unit>(selectedObj)) {
					Unit &unit = registry.get<Unit>(selectedObj);

					ImGui::BeginGroup();
					ImGui::Text("PV: %d", obj.life);
					ImGui::Text("AC: %d", unit.attack1.power);
					ImGui::Text("DC: %d", unit.attack1.distance);
					ImGui::Text("AE: %d", unit.attack2.power);
					ImGui::Text("DE: %d", unit.attack2.distance);
					ImGui::EndGroup();

					ImGui::SameLine();

					ImGui::BeginGroup();
					if (ImGui::ImageButtonAnim(factory.texManager.getRef(player.team + "_move"),
					                           factory.texManager.getRef(player.team + "_move"),
					                           factory.texManager.getRef(player.team + "_move_down"))) {
						std::cout << "move clicked " << std::endl;
					}

					ImGui::SameLine();
					if (ImGui::ImageButtonAnim(factory.texManager.getRef(player.team + "_attack"),
					                           factory.texManager.getRef(player.team + "_attack"),
					                           factory.texManager.getRef(player.team + "_attack_down"))) {
						std::cout << "attack clicked " << std::endl;

					}
					ImGui::EndGroup();

				}

				TechNode *pnode = factory.getTechNode(player.team, obj.name);
				if (pnode->children.size() > 0) {
					for (TechNode &node : pnode->children) {
						if (ImGui::ImageButtonAnim(factory.texManager.getRef(node.type + "_icon"),
						                           factory.texManager.getRef(node.type + "_icon"),
						                           factory.texManager.getRef(node.type + "_icon_down"))) {
							std::cout << "build clicked " << node.type << std::endl;
							switch (node.comp) {
							case TechComponent::Building:
								this->action = Action::Building;
								this->currentBuildType = node.type;
								break;
							case TechComponent::Character:
								if (this->spendResources(registry, factory, this->currentPlayer, player.resourceType, 10)) {
									for (sf::Vector2i p : this->tileAround(tile, 1)) {
										if (!map.objs.get(p.x, p.y)) {

											factory.createUnit(registry, this->currentPlayer, node.type, p.x, p.y);
											break;

										}
									}
								}
								break;
							case TechComponent::Resource:
								this->seedResources(registry, factory, player.resourceType, selectedObj);
								break;
							}
						}
						ImGui::SameLine();
					}
				}


			} else {
				if (this->selectedObjs.size() == 0) {
					TechNode *node = factory.getTechRoot(player.team);
					if (ImGui::ImageButtonAnim(factory.texManager.getRef(node->type + "_icon"),
					                           factory.texManager.getRef(node->type + "_icon"),
					                           factory.texManager.getRef(node->type + "_icon_down"))) {
						std::cout << "build clicked " << node->type << std::endl;
						this->action = Action::Building;
						this->currentBuildType = node->type;
//						this->objLayerUpdate(registry, factory, 0);
					}
				}
			}
			ImGui::End();

		}
		ImGui::PopStyleColor();
	}

	void testGui(entt::Registry<EntityID> &registry, EntityFactory &factory) {
		const float DISTANCE = 10.0f;
		static int corner = 0;
		ImVec2 window_pos = ImVec2((corner & 1) ? ImGui::GetIO().DisplaySize.x - DISTANCE : DISTANCE, (corner & 2) ? ImGui::GetIO().DisplaySize.y - DISTANCE : DISTANCE);
		ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
		ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.3f)); // Transparent background
		ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[2]);
		if (ImGui::Begin("Example: Fixed Overlay", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
		{
			ImGui::Text("Entities: %d", registry.size());
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

	void update(entt::Registry<EntityID> &registry, EntityFactory &factory, float dt) {
		this->updateEveryFrame(registry, factory, dt);
		this->currentTime += dt;
		if (this->currentTime < this->timePerTick) return;

		this->currentTime = 0.0;

		if (this->action == Action::Building && this->currentBuild == 0) {
			this->currentBuild = factory.createBuilding(registry, this->currentPlayer, this->currentBuildType, 8, 8, false);
			std::cout << "built " << this->currentBuild << std::endl;
		}

		this->updateCombat(registry, factory, dt);
		this->growResources(registry, factory, dt);
		this->updateObjsLayer(registry, factory, dt);
		this->updateTileLayer(registry, factory, dt);
	}

	void updateEveryFrame(entt::Registry<EntityID> &registry, EntityFactory &factory, float dt)
	{
		this->updateAnim(registry, factory, dt);
		this->updatePathfinding(registry, factory, dt);
	}

	void updateAnim(entt::Registry<EntityID> &registry, EntityFactory &factory, float dt) {
		auto view = registry.view<Tile>();
		for (EntityID entity : view) {
			Tile &tile = view.get(entity);

			AnimationHandler &currentAnim = tile.animHandlers[tile.state];

			/* Change the sprite to reflect the tile variant */
			currentAnim.changeAnim(tile.direction);

			/* Update the animation */
			currentAnim.update(dt);

			/* Update the sprite */
			tile.sprite.setTextureRect(currentAnim.bounds);

		}

	}

// HELPERS
	float approxDistance(sf::Vector2i p1, sf::Vector2i p2) {
		double dx = abs(p1.x - p2.x);
		double dy = abs(p1.y - p2.y);
		return 0.394 * (dx + dy) + 0.554 * std::max(dx, dy);
	}

	int getDirection(sf::Vector2i src, sf::Vector2i dst) {
		sf::Vector2i diff = dst - src;
		if (diff.x <= -1) {

			if (diff.y <= -1)
				return NorthWest;

			if (diff.y == 0)
				return West;

			if (diff.y >= 1)
				return SouthWest;
		}

		if (diff.x == 0) {
			if (diff.y <= -1)
				return North;

			if (diff.y == 0)
				return North;

			if (diff.y >= 1)
				return South;
		}

		if (diff.x >= 1) {
			if (diff.y <= -1)
				return NorthEast;

			if (diff.y == 0)
				return East;

			if (diff.y >= 1)
				return SouthEast;
		}

	}

	std::vector<sf::Vector2i> tileSurface(Tile &tile) {
		std::vector<sf::Vector2i> surface;
		for (int w = 0; w < tile.size.x; w++) {
			for (int h = 0; h < tile.size.y; h++) {
				int x = tile.pos.x + (w - tile.size.x / 2) + tile.offset.x;
				int y = tile.pos.y + (h - tile.size.y / 2) + tile.offset.y;
				if (x >= 0 && y >= 0 && x < map.width && y < map.height)
					surface.push_back(sf::Vector2i(x, y));
			}
		}
		return surface;
	}


	std::vector<sf::Vector2i> tileSurfaceExtended(Tile &tile) {
		std::vector<sf::Vector2i> surface;
		for (int w = -1; w < tile.size.x + 1; w++) {
			for (int h = -1; h < tile.size.y + 1; h++) {
				int x = tile.pos.x + (w - tile.size.x / 2) + tile.offset.x;
				int y = tile.pos.y + (h - tile.size.y / 2) + tile.offset.y;
				if (x >= 0 && y >= 0 && x < map.width && y < map.height)
					surface.push_back(sf::Vector2i(x, y));
			}
		}
		return surface;
	}

	std::vector<sf::Vector2i> tileAround(Tile &tile, int dist) {
		std::vector<sf::Vector2i> surface;
		for (int w = -dist; w < tile.size.x + dist; w++) {
			for (int h = -dist; h < tile.size.y + dist; h++) {
				if (w <= -1 || h <= -1 || w >= tile.size.x || h >= tile.size.y) {
					int x = tile.pos.x + (w - tile.size.x / 2) + tile.offset.x;
					int y = tile.pos.y + (h - tile.size.y / 2) + tile.offset.y;
					if (x >= 0 && y >= 0 && x < map.width && y < map.height)
						surface.push_back(sf::Vector2i(x, y));
				}
			}
		}
		return surface;
	}

	sf::Vector2i nearestTileAround(sf::Vector2i src, Tile &tile, int dist) {
		sf::Vector2i nearest(1024, 1024);

		for (int w = -dist; w < tile.size.x + dist; w++) {
			for (int h = -dist; h < tile.size.y + dist; h++) {
				if (w <= -1 || h <= -1 || w >= tile.size.x || h >= tile.size.y) {
					sf::Vector2i p;

					p.x = tile.pos.x + (w - tile.size.x / 2) + tile.offset.x;
					p.y = tile.pos.y + (h - tile.size.y / 2) + tile.offset.y;
					if (p.x >= 0 && p.y >= 0 && p.x < map.width && p.y < map.height) {
						if (this->approxDistance(src, p) < this->approxDistance(src, nearest))
							nearest = p;
					}
				}
			}
		}
		return nearest;
	}

	sf::Vector2i firstFreePosition(sf::Vector2i src) {
		sf::Vector2i fp;
		int dist = 1;
		while (dist < 16) {
			for (int w = -dist; w < dist + 1; w++) {
				for (int h = -dist; h < dist + 1; h++) {
					if (w == -dist || h == -dist || w == dist || h == dist) {
						int x = w + src.x;
						int y = h + src.y;
						if (x >= 0 && y >= 0 && x < map.width && y < map.height) {
							if (!map.objs.get(x, y))
								return sf::Vector2i(x, y);
						}
					}
				}
			}
			dist++;
		}
	}


	void updateTileLayer(entt::Registry<EntityID> &registry, EntityFactory &factory, float dt) {
		auto view = registry.persistent<Tile, Building, GameObject>();

		// update tile with building
		for (EntityID entity : view) {
			Tile &tile = view.get<Tile>(entity);
			Building &building = view.get<Building>(entity);
			GameObject &obj = view.get<GameObject>(entity);

			if (building.built) {
				for (sf::Vector2i p : this->tileSurfaceExtended(tile)) {
					if (obj.team == "rebel") {
						map.terrains.set(p.x, p.y, map.tiles["grass"]);
					} else {
						map.terrains.set(p.x, p.y, map.tiles["concrete"]);
					}
				}
			}
		}

		// update tile with resource
		auto resView = registry.persistent<Tile, Resource>();
		for (EntityID entity : resView) {
			Tile &tile = resView.get<Tile>(entity);
			Resource &resource = resView.get<Resource>(entity);

			for (sf::Vector2i p : this->tileSurface(tile)) {
				if (resource.type == ResourceType::Nature) {
					map.terrains.set(p.x, p.y, map.tiles["grass"]);
				} else {
					map.terrains.set(p.x, p.y, map.tiles["concrete"]);
				}
			}

		}
	}

	void growResources(entt::Registry<EntityID> &registry, EntityFactory &factory, float dt) {
		int natureResources = 0;
		int pollutionResources = 0;

		auto view = registry.persistent<Tile, Resource>();
		for (EntityID entity : view) {
			Resource &resource = view.get<Resource>(entity);
			Tile &tile = view.get<Tile>(entity);
			resource.grow += 0.1;

//				std::cout << "RESOURCE "<<entity<<" grow "<<resource.grow << std::endl;

			if (resource.grow > 10) {
				std::cout << "RESOURCE " << entity << " grow" << std::endl;
				resource.grow = 0.0;

				if (resource.level < 3) {
					resource.level++;
					if (resource.level == 1)
						factory.growedResource(registry, factory.resourceTypeName(resource.type), entity);
					else
						tile.animHandlers[tile.state].set(resource.level - 1);
				} else {
					// max
					this->seedResources(registry, factory, resource.type, entity);

				}
			}

			switch (resource.type) {
			case ResourceType::Nature:
				natureResources += resource.level;
				break;
			case ResourceType::Pollution:
				pollutionResources += resource.level;
				break;
			}

		}

		auto playerView = registry.view<Player>();
		for (EntityID entity : playerView) {
			Player &player = playerView.get(entity);
			if (player.team == "rebel")
				player.resources = natureResources;
			else
				player.resources = pollutionResources;
		}


	}

	void updateObjsLayer(entt::Registry<EntityID> &registry, EntityFactory &factory, float dt) {
		map.clearEntities();

		map.resources.fill();
		auto resView = registry.persistent<Tile, Resource>();

		for (EntityID entity : resView) {
			Tile &tile = resView.get<Tile>(entity);

			for (sf::Vector2i p : this->tileSurface(tile)) {
				map.resources.set(p.x, p.y, entity);
			}			

			map.addEntity(entity);
		}


		map.objs.fill();
		auto view = registry.persistent<Tile, GameObject>();

		for (EntityID entity : view) {
			Tile &tile = view.get<Tile>(entity);

			for (sf::Vector2i p : this->tileSurface(tile)) {
				map.objs.set(p.x, p.y, entity);
			}

			map.addEntity(entity);
		}

/*
		auto unitView = registry.persistent<Tile, Unit>();

		for (EntityID entity : unitView) {
			Tile &tile = unitView.get<Tile>(entity);
			Unit &unit = unitView.get<Unit>(entity);

			map.objs.set(unit.nextpos.x, unit.nextpos.y, entity);
		}
*/
		std::sort( map.entities.begin( ), map.entities.end( ), [&registry ]( const auto & lhs, const auto & rhs )
		{
			Tile &lht = registry.get<Tile>(lhs);
			Tile &rht = registry.get<Tile>(rhs);
			/*			if (lht.pos.y < rht.pos.y)
							return true;
						else if (lht.pos.y == rht.pos.y)
							return lht.pos.x < rht.pos.x;
						else
							return false;
			*/
			return (lht.pos.y < rht.pos.y);
		});

	}


	void updateTileDirection(Tile &tile, int cx, int cy, int nx, int ny) {
		tile.direction = this->getDirection(sf::Vector2i(cx, cy), sf::Vector2i(nx, ny));
		/*
		switch (nx - cx) {
		case -1:
			switch (ny - cy) {
			case -1:
				tile.direction = NorthWest;
				break;
			case 0:
				tile.direction = West;
				break;
			case 1:
				tile.direction = SouthWest;
				break;
			}
			break;
		case 0:
			switch (ny - cy) {
			case -1:
				tile.direction = North;
				break;
			case 0:
				break;
			case 1:
				tile.direction = South;
				break;
			}
			break;
		case 1:
			switch (ny - cy) {
			case -1:
				tile.direction = NorthEast;
				break;
			case 0:
				tile.direction = East;
				break;
			case 1:
				tile.direction = SouthEast;
				break;
			}
			break;

		}
		*/
	}

	void updateCombat(entt::Registry<EntityID> &registry, EntityFactory &entityFactory, float dt) {
		auto view = registry.persistent<Tile, Unit>();
		for (EntityID entity : view) {
			Tile &tile = view.get<Tile>(entity);
			Unit &unit = view.get<Unit>(entity);
			if (unit.destAttack && registry.valid(unit.destAttack)) {
				int dist = 1;
				if (unit.attack2.distance)
					dist = unit.attack2.distance;
				Tile &destTile = registry.get<Tile>(unit.destAttack);
				GameObject &destObj = registry.get<GameObject>(unit.destAttack);

				sf::Vector2i dpos = this->nearestTileAround(tile.pos, destTile, dist);
				if (tile.pos == dpos) {
					std::cout << "REALLY ATTACK " << entity << " " << destObj.life << std::endl;
					tile.direction = this->getDirection(tile.pos, destTile.pos);
					tile.state = "attack";
					destObj.life -= (unit.attack1.power * dt);
					if (destObj.life < 0)
						destObj.life = 0;

					if (destObj.life == 0) {
						tile.state = "idle";
						unit.destAttack = 0;

						destTile.state = "die";
					}
				} else {
					unit.destpos = dpos;
					unit.nopath = 0;
					std::cout << "GO ATTACK " << entity << " " << unit.destpos.x << "x" << unit.destpos.y << std::endl;
				}
			} else {
				unit.destAttack = 0;
			}
		}

		auto objView = registry.persistent<Tile, GameObject>();
		for (EntityID entity : objView) {
			Tile &tile = objView.get<Tile>(entity);
			GameObject &obj = objView.get<GameObject>(entity);
			if (obj.life == 0 && tile.animHandlers[tile.state].l >= 1) {
				registry.destroy(entity);
			}
		}

	}

	void updatePathfinding(entt::Registry<EntityID> &registry, EntityFactory &entityFactory, float dt) {
		auto view = registry.persistent<Tile, Unit>();
		for (EntityID entity : view) {
			Tile &tile = view.get<Tile>(entity);
			Unit &unit = view.get<Unit>(entity);

			if (tile.pos != unit.destpos) {
				int diffx = abs(tile.ppos.x - unit.nextpos.x * 32);
				int diffy = abs(tile.ppos.y - unit.nextpos.y * 32);
				if (diffx >= 0 && diffx <= 2 && diffy >= 0 && diffy <= 2) {
					tile.pos = unit.nextpos;
//							map.objs.set(tile.pos.x, tile.pos.y, entity);
					tile.ppos = sf::Vector2f(tile.pos) * (float)32.0;

					if (tile.pos != unit.destpos) {

						JPS::PathVector path;
						bool found = JPS::findPath(path, map, tile.pos.x, tile.pos.y, unit.destpos.x, unit.destpos.y, 1);

						if (found) {
							int cx = tile.pos.x;
							int cy = tile.pos.y;

							int nx = path.front().x;
							int ny = path.front().y;

//							map.objs.set(nx, ny, entity);

							unit.nextpos.x = nx;
							unit.nextpos.y = ny;

							this->updateTileDirection(tile, cx, cy, nx, ny);
							tile.state = "move";

							std::cout << "Pathfinding: " << entity << " next pos " << nx << "x" << ny << "(" << nx - cx << "x" << ny - cy << ")" << std::endl;
						} else {
							std::cout << "Pathfinding: " << entity << " no path found" << std::endl;
							tile.state = "idle";
							unit.nopath++;
							if (unit.nopath > 16) {
								sf::Vector2i fp = this->firstFreePosition(unit.destpos);
								std::cout << "first free pos " << fp.x << "x" << fp.y << std::endl;
								unit.destpos = fp;
								unit.nopath = 0;
							}
						}
					} else {
						std::cout << "Pathfinding: " << entity << " at destination" << std::endl;
						tile.state = "idle";
					}
				} else {
					float speed = (float)unit.speed / 2.0;
					switch (tile.direction) {
					case North:
						tile.ppos.y -= speed;
						break;
					case NorthEast:
						tile.ppos.x += speed;
						tile.ppos.y -= speed;
						break;
					case East:
						tile.ppos.x += speed;
						break;
					case SouthEast:
						tile.ppos.x += speed;
						tile.ppos.y += speed;
						break;
					case South:
						tile.ppos.y += speed;
						break;
					case NorthWest:
						tile.ppos.x -= speed;
						tile.ppos.y -= speed;
						break;
					case West:
						tile.ppos.x -= speed;
						break;
					case SouthWest:
						tile.ppos.x -= speed;
						tile.ppos.y += speed;
						break;
					}

//							map.objs.set(unit.nextpos.x, unit.nextpos.y, entity);

				}
			}
		}

	}

};

