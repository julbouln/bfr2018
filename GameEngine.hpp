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

		factory.createUnit(registry, "zork", 10, 10);
		factory.createUnit(registry, "zork", 10, 11);
		factory.createUnit(registry, "zork", 10, 12);
		factory.createUnit(registry, "zork", 11, 10);
		factory.createUnit(registry, "zork", 11, 11);
		factory.createUnit(registry, "zork", 11, 12);
		factory.createUnit(registry, "zork", 12, 10);
		factory.createUnit(registry, "zork", 12, 11);
		factory.createUnit(registry, "zork", 12, 12);

		factory.createBuilding(registry, "taverne", 16, 10, true);

//		map.objs.set(10, 10, factory.createUnit(registry, "zork", 10, 10));
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

					EntityID entity = map.objs.get(gameMapPos.x, gameMapPos.y);
					if (entity) {
						std::cout << "SELECT " << entity << std::endl;
						this->selectedObjs.clear();
						this->selectedObjs.push_back(entity);

					} else {
						this->selectedObjs.clear();
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
						for (EntityID selectedObj : this->selectedObjs) {
							if (registry.has<Unit>(selectedObj)) {
								Tile &tile = registry.get<Tile>(selectedObj);
								Unit &unit = registry.get<Unit>(selectedObj);
								unit.nextpos = tile.pos;
								unit.destpos = sf::Vector2i(gameMapPos);
								tile.ppos = sf::Vector2f(tile.pos) * (float)32.0;
							}
						}
					}
				}
			}
		}
		break;

		}
	}

	void drawTileLayer(entt::Registry<EntityID> &registry, EntityFactory &factory, TileLayer &layer, sf::RenderWindow &window, float dt) {
		for (int y = 0; y < layer.height; ++y)
		{
			for (int x = 0; x < layer.width; ++x)
			{
				EntityID ent = layer.get(x, y);
//					std::cout << "LAY "<< ent << std::endl;
				if (ent) {
//					std::cout << " draw " << ent << std::endl;
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


	void drawObjLayer(entt::Registry<EntityID> &registry, EntityFactory &factory, ObjLayer &layer, sf::RenderWindow &window, float dt) {
		/*		for (int y = 0; y < layer.height; ++y)
				{
					for (int x = 0; x < layer.width; ++x)
					{
						EntityID ent = layer.get(x, y);
		*/
		for (EntityID ent : layer.entities) {
//					std::cout << "LAY "<< ent << std::endl;
//			if (ent) {
//					std::cout << " draw " << ent << std::endl;
			Tile &tile = registry.get<Tile>(ent);

			sf::Vector2f pos;
			pos.x = tile.ppos.x - tile.psize.x / 2 + 16;
			pos.y = tile.ppos.y - tile.psize.y / 2;

			tile.sprite.setPosition(pos);

			/* Draw the tile */
			window.draw(tile.sprite);

//			}
		}
		/*			}

				}
				*/
	}


	void draw(entt::Registry<EntityID> &registry, EntityFactory &factory, sf::RenderWindow &window, float dt) {
		if (this->markUpdateObjLayer) {
			this->updateObjLayer(registry, factory, 0);
			this->markUpdateObjLayer = false;
		}

		this->drawTileLayer(registry, factory, map.terrains, window, dt);
		this->drawObjLayer(registry, factory, map.objs, window, dt);

		for (EntityID selectedObj : this->selectedObjs) {
			Tile &tile = registry.get<Tile>(selectedObj);
			sf::RectangleShape rectangle;

			sf::Vector2f pos;
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
		this->actionGui(registry, factory);
	}


	void actionGui(entt::Registry<EntityID> &registry, EntityFactory &factory) {
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
					if (ImGui::ImageButtonAnim(factory.texManager.getRef("rebel_move"),
					                           factory.texManager.getRef("rebel_move"),
					                           factory.texManager.getRef("rebel_move_down"))) {
						std::cout << "move clicked " << std::endl;
					}

					ImGui::SameLine();
					if (ImGui::ImageButtonAnim(factory.texManager.getRef("rebel_attack"),
					                           factory.texManager.getRef("rebel_attack"),
					                           factory.texManager.getRef("rebel_attack_down"))) {
						std::cout << "attack clicked " << std::endl;

					}
					ImGui::EndGroup();

				}

				TechNode *pnode = factory.getTechNode("rebel", obj.name);
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
							}
						}
						ImGui::SameLine();
					}
				}


			} else {
				if (this->selectedObjs.size() == 0) {
					TechNode *node = factory.getTechNode("rebel", "taverne");
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
		if (ImGui::Begin("Example: Fixed Overlay", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
		{
			ImGui::Text("Entities: %d", registry.size());
			ImGui::Text("Simple overlay\nin the corner of the screen.\n(right-click to change position)");
			ImGui::Separator();
			ImGui::Text("Mouse Position: (%.1f,%.1f)", ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y);

//			if (ImGui::ImageButtonBlend(factory.texManager.getRef("zork_icon"))) {
//			}

//			if (ImGui::ImageButtonWithText(factory.texManager.getRef("zork_icon"),"test",ImVec2(48,48),0,ImVec4(0,0,0,0),ImVec4(255,255,255,255))) {
//			}

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

	}

	void update(entt::Registry<EntityID> &registry, EntityFactory &factory, float dt) {
		this->updateEveryFrame(registry, factory, dt);
		this->currentTime += dt;
		if (this->currentTime < this->timePerTick) return;

		this->currentTime = 0.0;

		if (this->action == Action::Building && this->currentBuild == 0) {
			this->currentBuild = factory.createBuilding(registry, this->currentBuildType, 8, 8, false);
			std::cout << "built " << this->currentBuild << std::endl;
		}

		this->updateObjLayer(registry, factory, dt);
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


	std::vector<sf::Vector2i> tileSurface(Tile &tile) {
		std::vector<sf::Vector2i> surface;
		for (int w = 0; w < tile.size.x; w++) {
			for (int h = 0; h < tile.size.y; h++) {
				surface.push_back(sf::Vector2i(tile.pos.x + (w - tile.size.x / 2), tile.pos.y + (h - tile.size.y / 2)));
			}
		}
		return surface;
	}


	std::vector<sf::Vector2i> tileSurfaceExtended(Tile &tile) {
		std::vector<sf::Vector2i> surface;
		for (int w = -1; w < tile.size.x + 1; w++) {
			for (int h = -1; h < tile.size.y + 1; h++) {
				surface.push_back(sf::Vector2i(tile.pos.x + (w - tile.size.x / 2), tile.pos.y + (h - tile.size.y / 2)));
			}
		}
		return surface;
	}

	std::vector<sf::Vector2i> tileAround(Tile &tile, int dist) {
		std::vector<sf::Vector2i> surface;
		for (int w = -dist; w < tile.size.x + dist; w++) {
			for (int h = -dist; h < tile.size.y + dist; h++) {
				if(w <= -1 || h <= -1 || w>=tile.size.x || h>=tile.size.y)
					surface.push_back(sf::Vector2i(tile.pos.x + (w - tile.size.x / 2), tile.pos.y + (h - tile.size.y / 2)));
			}
		}
		return surface;
	}


	void updateTileLayer(entt::Registry<EntityID> &registry, EntityFactory &factory, float dt) {
		auto view = registry.persistent<Tile, Building, GameObject>();

		for (EntityID entity : view) {
			Tile &tile = view.get<Tile>(entity);
			Building &building = view.get<Building>(entity);
			GameObject &obj = view.get<GameObject>(entity);

			if (building.built && obj.team == "rebel") {
				for (sf::Vector2i p : this->tileSurfaceExtended(tile)) {
					map.terrains.set(p.x, p.y, map.tiles["grass"]);
				}
			}
		}
	}

	void updateObjLayer(entt::Registry<EntityID> &registry, EntityFactory &factory, float dt) {
		map.objs.fill();
		auto view = registry.persistent<Tile, GameObject>();

		for (EntityID entity : view) {
			Tile &tile = view.get<Tile>(entity);
//			Unit &unit = unitView.get<Unit>(entity);

			for (sf::Vector2i p : this->tileSurface(tile)) {
				map.objs.set(p.x, p.y, entity);
			}

//			std::cout << "SET "<<entity << " "<<tile.pos.x << "x"<< tile.pos.y<< " "<<map.objs.entitiesGrid.size() << std::endl;
			map.objs.add(entity);
		}


		sort( map.objs.entities.begin( ), map.objs.entities.end( ), [&registry ]( const auto & lhs, const auto & rhs )
		{
			Tile &lht = registry.get<Tile>(lhs);
			Tile &rht = registry.get<Tile>(rhs);
			return lht.pos.y < rht.pos.y;
		});

	}

	void updateTileDirection(Tile &tile, int cx, int cy, int nx, int ny) {
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
	}


	void updatePathfinding(entt::Registry<EntityID> &registry, EntityFactory &entityFactory, float dt) {
		auto view = registry.persistent<Tile, Unit>();
		for (EntityID entity : view) {
			Tile &tile = view.get<Tile>(entity);
			Unit &unit = view.get<Unit>(entity);

			if (tile.pos != unit.destpos) {
//				if ((trunc((float)tile.ppos.x / 32.0) - unit.nextpos.x)==0 && (trunc((float)tile.ppos.y / 32.0) - unit.nextpos.y) == 0) {
				int diffx = abs(tile.ppos.x - unit.nextpos.x * 32);
				int diffy = abs(tile.ppos.y - unit.nextpos.y * 32);
				if (diffx >= 0 && diffx <= 2 && diffy >= 0 && diffy <= 2) {
					if (tile.pos != unit.nextpos) {
						if (!map.objs.get(unit.nextpos.x, unit.nextpos.y)) {
							tile.pos = unit.nextpos;
						}
//						map.objs.set(tile.pos.x, tile.pos.y, entity);
					}
					tile.ppos = sf::Vector2f(tile.pos) * (float)32.0;

					if (tile.pos != unit.destpos) {

						JPS::PathVector path;
						bool found = JPS::findPath(path, map, tile.pos.x, tile.pos.y, unit.destpos.x, unit.destpos.y, 1);

						if (found) {
							int cx = tile.pos.x;
							int cy = tile.pos.y;

							int nx = path.front().x;
							int ny = path.front().y;


							unit.nextpos.x = nx;
							unit.nextpos.y = ny;

							this->updateTileDirection(tile, cx, cy, nx, ny);
							tile.state = "move";

							std::cout << "Pathfinding: " << entity << " next pos " << nx << "x" << ny << "(" << nx - cx << "x" << ny - cy << ")" << std::endl;
						} else {
							std::cout << "Pathfinding: " << entity << " no path found" << std::endl;
							tile.state = "idle";
							unit.destpos = tile.pos;
//							map.objs.set(tile.pos.x, tile.pos.y, entity);
						}
					} else {
						std::cout << "Pathfinding: " << entity << " at destination" << std::endl;
						tile.state = "idle";
//						map.objs.set(tile.pos.x, tile.pos.y, entity);
					}

					map.objs.set(tile.pos.x, tile.pos.y, entity);

				} else {
					float speed = (float)unit.speed / 4.0;
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

//					std::cout << "Pathfinding: " << entity << " intermediate " << tile.ppos.x << "x" << tile.ppos.y << " " << tile.ppos.x / 32 << "x" << tile.ppos.y / 32 << ": " << unit.nextpos.x << "x" << unit.nextpos.y << std::endl;
//					std::cout << "Pathfinding: " << entity << " diff " << (tile.ppos.x - unit.nextpos.x * 32) << "x" << (tile.ppos.y - unit.nextpos.y * 32) << std::endl;
				}
			}
		}

	}

};

