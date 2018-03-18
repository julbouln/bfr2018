#include "InterfaceSystem.hpp"


void InterfaceSystem::init() {
	GameController &controller = this->vault->registry.get<GameController>();
	Player &player = this->vault->registry.get<Player>(controller.currentPlayer);
	if (player.team != "neutral") {
		iface.setTexture(this->vault->factory.getTex("interface_" + player.team));
		box.setTexture(this->vault->factory.getTex("box_" + player.team));
		box_w = this->vault->factory.getTex("box_" + player.team).getSize().x;
		minimap_bg.setTexture(this->vault->factory.getTex("minimap_" + player.team));
		minimap_bg_h = this->vault->factory.getTex("minimap_" + player.team).getSize().y;
		indice.setTexture(this->vault->factory.getTex("indice_" + player.team));
		indice_bg.setTexture(this->vault->factory.getTex("indice_bg_" + player.team));
	}
};

void InterfaceSystem::update(float dt) {
	// remove entity from selected is not valid anymore
	GameController &controller = this->vault->registry.get<GameController>();
	std::vector<EntityID> newSelectedObjs;
	for (EntityID entity : controller.selectedObjs) {
		if (this->vault->registry.valid(entity))
			newSelectedObjs.push_back(entity);
	}
	controller.selectedObjs = newSelectedObjs;
};

void InterfaceSystem::draw(sf::RenderWindow &window, sf::IntRect clip, float dt) {

	iface.setPosition(sf::Vector2f(0, 0));
	iface.setScale(this->scaleX(), this->scaleY());
	window.draw(iface);

	minimap_bg.setPosition(sf::Vector2f(0, (600 - minimap_bg_h) * this->scaleY()));
	minimap_bg.setScale(this->scaleX(), this->scaleY());
	window.draw(minimap_bg);

	box.setPosition(sf::Vector2f((800 - box_w) * this->scaleX(), (600 - 136) * this->scaleY()));
	box.setScale(this->scaleX(), this->scaleY());
	window.draw(box);

	this->menuGui();
	this->gameStateGui();
	this->actionGui();
}

void InterfaceSystem::addSelected(EntityID entity) {
	GameController &controller = this->vault->registry.get<GameController>();
	if (entity) {
		GameObject &obj = this->vault->registry.get<GameObject>(entity);

		if (obj.player == controller.currentPlayer) {
			if (this->vault->registry.has<Unit>(entity)) {
				Unit &unit = this->vault->registry.get<Unit>(entity);
				this->playRandomUnitSound(entity, "select");
			}
			controller.selectedObjs.push_back(entity);
		}
	}
}

void InterfaceSystem::clearSelected() {
	GameController &controller = this->vault->registry.get<GameController>();

	if (controller.action == Action::Select) {
		controller.selectionStart = sf::Vector2f(0, 0);
		controller.selectionEnd = sf::Vector2f(0, 0);
		controller.action = Action::None;
	}
}

void InterfaceSystem::menuGui() {
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // Transparent background
	if (ImGui::Begin("Menu", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
	{

		if (ImGui::ImageButtonAnim(this->vault->factory.texManager.getRef("menu_button"),
		                           this->vault->factory.texManager.getRef("menu_button"),
		                           this->vault->factory.texManager.getRef("menu_button_down"))) {
			this->vault->dispatcher.trigger<GameStageChange>(NextStageStr("play_menu"));

		}
		ImGui::End();
	}
	ImGui::PopStyleColor();
}


void InterfaceSystem::gameStateGui() {
	GameController &controller = this->vault->registry.get<GameController>();
	Player &player = this->vault->registry.get<Player>(controller.currentPlayer);
	float leftDist = 192.0f * this->scaleX();
	float topDist = 8.0f * this->scaleY();

	ImVec2 window_pos = ImVec2(leftDist, topDist);
	ImVec2 window_pos_pivot = ImVec2(0.0f, 0.0f);
	ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // Transparent background
	if (ImGui::Begin("State", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
	{
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0, 255, 0, 255));
		ImGui::ProgressBar((float)player.resources / this->resourcesVictory(), ImVec2(200.0f * this->scaleX(), 0.0f), "");
		ImGui::PopStyleColor();
		if (ImGui::IsItemHovered()) {
			ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
			ImGui::BeginTooltip();
			ImGui::Image(this->vault->factory.getTex(player.resourceType + "_cost")); ImGui::SameLine();
			ImGui::Text("%d", player.resources);
			ImGui::EndTooltip();
			ImGui::PopFont();
		}

		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(255, 0, 0, 255));
		ImGui::ProgressBar((float)player.butchery / this->butcheryVictory(), ImVec2(200.0f * this->scaleX(), 0.0f), "");
		ImGui::PopStyleColor();

		ImGui::End();
	}
	ImGui::PopStyleColor();
}

void InterfaceSystem::constructionProgressGui(EntityID consEnt) {
	GameController &controller = this->vault->registry.get<GameController>();
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
				controller.action = Action::Build;
				controller.currentBuild = this->vault->factory.finishBuilding(this->vault->registry, consEnt, controller.currentPlayer, 8, 8, false);
			}
			ImGui::SameLine();
		}
	}
}

void InterfaceSystem::actionGui() {
	GameController &controller = this->vault->registry.get<GameController>();
	Player &player = this->vault->registry.get<Player>(controller.currentPlayer);

	if (player.team != "neutral") {
		float uiX = 590.0f * this->scaleX();
		float uiHeight = 124.0f * this->scaleY();
		float uiWidth = 250.0f * this->scaleX();
		float uiLWidth = 300.0f * this->scaleX();
		float uiRWidth = 200.0f * this->scaleX();

		ImVec2 window_pos = ImVec2(uiX, ImGui::GetIO().DisplaySize.y - uiHeight);
		ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(uiWidth, uiHeight), ImGuiCond_Always);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // Transparent background
		if (ImGui::Begin("Actions", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
		{
			// only one object selected
			if (controller.selectedObjs.size() == 1) {
				EntityID selectedObj = controller.selectedObjs[0];

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
					if (ImGui::ImageButtonAnim(this->vault->factory.texManager.getRef("move"),
					                           this->vault->factory.texManager.getRef("move"),
					                           this->vault->factory.texManager.getRef("move_down"))) {
						controller.action = Action::Move;
					}

					ImGui::SameLine();
					if (ImGui::ImageButtonAnim(this->vault->factory.texManager.getRef("attack"),
					                           this->vault->factory.texManager.getRef("attack"),
					                           this->vault->factory.texManager.getRef("attack_down"))) {
						controller.action = Action::Attack;
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
										if (this->trainUnit(node.type, controller.currentPlayer, selectedObj)) {
//											this->markUpdateLayer = true;
										}
										break;
									case TechComponent::Resource:
										this->seedResources(player.resourceType, selectedObj);
										break;
									}
								}
								if (ImGui::IsItemHovered()) {
									switch (node.comp) {
									case TechComponent::Building: {
										ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
										ImGui::BeginTooltip();
										ImGui::Image(this->vault->factory.getTex("time")); ImGui::SameLine();
										ImGui::Text("%d", (int)this->vault->factory.buildTime(node.type));
										ImGui::EndTooltip();
										ImGui::PopFont();
									}
									break;
									case TechComponent::Character: {
										ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
										ImGui::BeginTooltip();
										ImGui::Image(this->vault->factory.getTex(player.resourceType + "_cost")); ImGui::SameLine();
										ImGui::Text("%d", (int)this->vault->factory.trainCost(node.type));
										ImGui::EndTooltip();
										ImGui::PopFont();
									}
									break;
									case TechComponent::Resource: {
										ImGui::SetTooltip("Plant resources");
									}
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
				// group selected
				if (controller.selectedObjs.size() > 0) {

					if (this->vault->factory.texManager.hasRef(player.team + "_face")) {
						ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
						ImGui::BeginGroup();
						ImGui::ImageWithText(this->vault->factory.texManager.getRef(player.team + "_face"), "%d", controller.selectedObjs.size());
						ImGui::EndGroup();
						ImGui::PopFont();
					}

					ImGui::BeginGroup();
					if (ImGui::ImageButtonAnim(this->vault->factory.texManager.getRef("move"),
					                           this->vault->factory.texManager.getRef("move"),
					                           this->vault->factory.texManager.getRef("move_down"))) {
						controller.action = Action::Move;
					}

					ImGui::SameLine();
					if (ImGui::ImageButtonAnim(this->vault->factory.texManager.getRef("attack"),
					                           this->vault->factory.texManager.getRef("attack"),
					                           this->vault->factory.texManager.getRef("attack_down"))) {
						controller.action = Action::Attack;
					}
					ImGui::EndGroup();
				} else { // nothing selected
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
						if (ImGui::IsItemHovered()) {
							ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
							ImGui::BeginTooltip();
							ImGui::Image(this->vault->factory.getTex("time")); ImGui::SameLine();
							ImGui::Text("%d", (int)this->vault->factory.buildTime(node->type));
							ImGui::EndTooltip();
							ImGui::PopFont();
						}
					}
				}
			}
			ImGui::End();
		}
		ImGui::PopStyleColor();
	}
}


void InterfaceSystem::debugGui(sf::RenderWindow &window, sf::View &view, int *gameSpeed, float dt) {
	GameController &controller = this->vault->registry.get<GameController>();
	sf::Vector2f gamePos = (window.mapPixelToCoords(sf::Mouse::getPosition(window), view));
	sf::Vector2f gameMapPos = gamePos;
	gameMapPos.x /= 32.0;
	gameMapPos.y /= 32.0;

	const float DISTANCE = 10.0f;
	ImVec2 window_pos = ImVec2((controller.debugCorner & 1) ? ImGui::GetIO().DisplaySize.x - DISTANCE : DISTANCE, (controller.debugCorner & 2) ? ImGui::GetIO().DisplaySize.y - DISTANCE : DISTANCE);
	ImVec2 window_pos_pivot = ImVec2((controller.debugCorner & 1) ? 1.0f : 0.0f, (controller.debugCorner & 2) ? 1.0f : 0.0f);
	ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.3f)); // Transparent background
	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[3]);
	if (ImGui::Begin("Debug", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
	{
		ImGui::Text("FPS: %.2f", 1 / dt);
		ImGui::Text("Total Entities: %d", (int)this->vault->registry.size());
//		ImGui::Text("Drawn Entities: %d", (int)drawMap.entitiesDrawList.size());
//		ImGui::Checkbox("Debug layer", &drawMap.showDebugLayer);

		ImGui::Text("Speed"); ImGui::SameLine();
		ImGui::RadioButton("0", gameSpeed, 0); ImGui::SameLine();
		ImGui::RadioButton("x1", gameSpeed, 1); ImGui::SameLine();
		ImGui::RadioButton("x4", gameSpeed, 4); ImGui::SameLine();
		ImGui::RadioButton("x16", gameSpeed, 16);

		if (controller.selectedDebugObj) {
			EntityID selectedObj = controller.selectedDebugObj;
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
					ImGui::Text("Direction: %dx%d", unit.direction.x, unit.direction.y);
					ImGui::Text("Dest pos: %dx%d", unit.destpos.x, unit.destpos.y);
					ImGui::Text("Dest attack: %d", (int)unit.targetEnt);
					ImGui::Text("Velocity: %.2fx%.2f", unit.velocity.x, unit.velocity.y);
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
		ImGui::Text("Game Position: (%.1f,%.1f)", gamePos.x, gamePos.y);
		ImGui::Text("Game map position: (%.1f,%.1f)", gameMapPos.x, gameMapPos.y);

		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::MenuItem("Top-left", NULL, controller.debugCorner == 0)) controller.debugCorner = 0;
			if (ImGui::MenuItem("Top-right", NULL, controller.debugCorner == 1)) controller.debugCorner = 1;
			if (ImGui::MenuItem("Bottom-left", NULL, controller.debugCorner == 2)) controller.debugCorner = 2;
			if (ImGui::MenuItem("Bottom-right", NULL, controller.debugCorner == 3)) controller.debugCorner = 3;
			ImGui::EndPopup();
		}
		ImGui::End();
	}
	ImGui::PopStyleColor();
	ImGui::PopFont();
}

