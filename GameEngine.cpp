#include "GameEngine.hpp"

GameEngine::GameEngine(Game *game, unsigned int mapWidth, unsigned int mapHeight, std::string playerTeam) {
	this->game = game;
	this->init();
	this->generate(mapWidth, mapHeight, playerTeam);
	this->zoomLevel = 1.0;
}

GameEngine::~GameEngine() {
	this->setGameSpeed(1.0);
	// FIXME: registry must actually resides in GameEngine instead of game
	this->vault->dispatcher.update(); // to achieve a coherent state ?
	this->vault->registry.reset();
//	delete this->map;
}

void GameEngine::receive(const GameStageChange &event) {
	this->nextStage = event.nextStage;
	fadeOut();
}

void GameEngine::init() {
	this->timePerTick = 0.1;
	this->currentTime = 0.0;
	this->ticks = 0;
	this->markUpdateLayer = false;

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

	sf::Vector2f pos = sf::Vector2f(this->game->window.getSize());
	guiView.setSize(pos);
	gameView.setSize(pos);
	pos *= 0.5f;
	guiView.setCenter(pos);
	gameView.setCenter(pos);

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

	this->vault->dispatcher.connect<GameStageChange>(this);
}

void GameEngine::reset() {
	this->fadeIn();
	this->nextStage = 0;
}

void GameEngine::fadeOutCallback() {
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

void GameEngine::setVaults(GameVault *vault) {
	emptyEntity = vault->registry.create(); // create Entity 0 as special empty Entity
	gameEntity = vault->registry.create();

	this->setVault(vault);

	this->vault->registry.attach<Map>(gameEntity);

	this->map = &this->vault->registry.get<Map>();
	//new Map();

	// set shared systems
	gameGenerator.setShared(vault, this->map, this->width, this->height);
	tileAnim.setShared(vault, this->map, this->width, this->height);
	time.setShared(vault, this->map, this->width, this->height);
	resources.setShared(vault, this->map, this->width, this->height);
	drawMap.setShared(vault, this->map, this->width, this->height);
	minimap.setShared(vault, this->map, this->width, this->height);
	mapLayers.setShared(vault, this->map, this->width, this->height);
	construction.setShared(vault, this->map, this->width, this->height);
	pathfinding.setShared(vault, this->map, this->width, this->height);
	steering.setShared(vault, this->map, this->width, this->height);
	combat.setShared(vault, this->map, this->width, this->height);
	victory.setShared(vault, this->map, this->width, this->height);
	sound.setShared(vault, this->map, this->width, this->height);
	fx.setShared(vault, this->map, this->width, this->height);
	deletion.setShared(vault, this->map, this->width, this->height);
	ai.setShared(vault, this->map, this->width, this->height);
	interface.setShared(vault, this->map, this->width, this->height);
}

void GameEngine::centerMapView(sf::Vector2i position) {
	this->gameView.setCenter(sf::Vector2f(position) * 32.0f);
}

void GameEngine::generate(unsigned int mapWidth, unsigned int mapHeight, std::string playerTeam) {
	EntityID playerEnt = gameGenerator.generate(mapWidth, mapHeight, playerTeam);

	this->vault->registry.attach<GameController>(gameEntity);
	GameController &controller = this->vault->registry.get<GameController>();
	controller.currentPlayer = playerEnt;

	mapLayers.init();
	drawMap.init();
	ai.init();
	interface.init();
	deletion.init();
	minimap.init();
	victory.init();
	pathfinding.init();
	combat.init();
	tileAnim.init();
	fx.init();
	sound.init();

	Player &player = this->vault->registry.get<Player>(controller.currentPlayer);
	if (player.team != "neutral") {
		this->centerMapView(player.initialPos);
	} else {
		this->centerMapView(sf::Vector2i(this->map->width / 2, this->map->height / 2));
	}

//		EntityID pEnt = this->emitEffect("pluit", sf::Vector2f(this->map->width / 2 * 32.0, 1.0));
//		ParticleEffect &effect = this->vault->registry.get<ParticleEffect>(pEnt);
}

sf::IntRect GameEngine::viewClip() {
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

void GameEngine::draw(float dt) {
	GameController &controller = this->vault->registry.get<GameController>();

	this->game->window.setView(this->gameView);
	sf::IntRect clip = this->viewClip();

	drawMap.draw(this->game->window, clip, dt);
	fx.draw(this->game->window, clip, dt);

	drawMap.drawFogTileMap(this->game->window, dt);

	// draw selected
	for (EntityID selectedObj : controller.selectedObjs) {
		Tile &tile = this->vault->registry.get<Tile>(selectedObj);

		sf::Vector2f pos = this->tileDrawPosition(tile);

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

	if (controller.action == Action::Selecting) {
		sf::RectangleShape rectangle;
		rectangle.setSize(controller.selectionEnd - controller.selectionStart);
		rectangle.setFillColor(sf::Color(0x00, 0x00, 0x00, 0x00));
		rectangle.setOutlineColor(sf::Color::Blue);
		rectangle.setOutlineThickness(2);
		rectangle.setPosition(controller.selectionStart);

		this->game->window.draw(rectangle);
	}

	if (controller.currentBuild) {
		std::vector<sf::Vector2i> restricted = this->canBuild(controller.currentPlayer, controller.currentBuild);
		sf::Sprite forbid(this->vault->factory.getTex("forbid"));
		forbid.setTextureRect(sf::IntRect(0, 0, 20, 20));
		for (sf::Vector2i const &p : restricted) {
			sf::Vector2f sp(p.x * 32, p.y * 32);
			forbid.setPosition(sp);
			this->game->window.draw(forbid);
		}
	}

	victory.draw(this->game->window, dt);

	this->game->window.setView(this->guiView);

	if (controller.showDebugWindow) {
		interface.debugGui(this->game->window, this->gameView, &gameSpeed, dt);
	}

	this->setGameSpeed(gameSpeed);

	this->guiPushStyles();
	interface.draw(this->game->window, clip, dt);
	this->guiPopStyles();
	ImGui::SFML::Render(this->game->window);

	minimap.draw(this->game->window, dt);
	minimap.drawClip(this->game->window, this->gameView, clip, dt);

	this->updateFading();

	sf::Vector2f viewPos = this->gameView.getCenter();
	sf::Listener::setPosition(viewPos.x / 32.0, 0.f, viewPos.y / 32.0);
}

void GameEngine::setGameSpeed(float factor) {
	if (factor == 0.0) {
		this->game->window.setFramerateLimit(30);
		this->timePerTick = FLT_MAX;
	} else {
		this->game->window.setFramerateLimit(30 * factor);
		this->timePerTick = 0.1 / factor;
	}
}

void GameEngine::updatePlayers(float dt) {
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

void GameEngine::updateHundred(float dt) {
	GameController &controller = this->vault->registry.get<GameController>();
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

			if (entity == controller.currentPlayer) {
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

void GameEngine::updateDecade(float dt) {
	victory.update(dt);
	minimap.update(dt);
	combat.updateFront(dt);
}

void GameEngine::updateEveryFrame(float dt)
{
	if (this->markUpdateLayer) {
		this->mapLayers.updateObjsLayer(0);
		this->markUpdateLayer = false;
	}

	this->time.update(dt);
	this->tileAnim.update(dt);
	this->steering.update(dt);

	this->sound.update(dt);

	this->updateMoveView(dt);

	this->fx.update(dt);
}

void GameEngine::update(float dt) {
	GameController &controller = this->vault->registry.get<GameController>();
	float updateDt = dt;

	if (this->timePerTick == FLT_MAX) return;

	this->updateEveryFrame(dt);

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

	this->resources.update(updateDt);

	this->deletion.update(updateDt);
	this->mapLayers.update(updateDt);

	this->drawMap.update(updateDt);
	this->map->markUpdateClear();

	this->mapLayers.updateFog(dt);

	ai.update(updateDt);

	interface.update(updateDt);
}

void GameEngine::updateMoveView(float dt) {
	GameController &controller = this->vault->registry.get<GameController>();

	sf::Vector2f center = this->gameView.getCenter();
	float mov = 160.0 * dt;
	switch (controller.moveView) {
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

void GameEngine::handleEvent(sf::Event & event) {
	GameController &controller = this->vault->registry.get<GameController>();
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
			controller.moveView = MoveView::DontMove;
			break;
		case sf::Event::KeyPressed:
		{
			controller.moveView = MoveView::DontMove;

			if (event.key.code == sf::Keyboard::Space) {
				// pause/unpause
				if (this->gameSpeed == 0) {
					this->gameSpeed = 1;
				} else {
					this->gameSpeed = 0;
				}
			}
			if (event.key.code == sf::Keyboard::Left)
				controller.moveView = MoveView::MoveWest;
			if (event.key.code == sf::Keyboard::Right)
				controller.moveView = MoveView::MoveEast;
			if (event.key.code == sf::Keyboard::Up)
				controller.moveView = MoveView::MoveNorth;
			if (event.key.code == sf::Keyboard::Down)
				controller.moveView = MoveView::MoveSouth;
			if (event.key.code == sf::Keyboard::Tab) {
				// debug window
				if (controller.showDebugWindow)
					controller.showDebugWindow = false;
				else
					controller.showDebugWindow = true;
			}
		}
		break;
		case sf::Event::MouseMoved:
		{
			controller.selectionEnd = gamePos;

			if (controller.action == Action::Building)
			{
				Tile &tile = this->vault->registry.get<Tile>(controller.currentBuild);
				tile.pos = sf::Vector2i(gameMapPos);
				tile.ppos = sf::Vector2f(tile.pos) * (float)32.0;
			}

			controller.moveView = MoveView::DontMove;

			if (mousePos.x < 32)
				controller.moveView = MoveView::MoveWest;
			if (mousePos.x > this->width - 32)
				controller.moveView = MoveView::MoveEast;
			if (mousePos.y < 32)
				controller.moveView = MoveView::MoveNorth;
			if (mousePos.y > this->height - 32)
				controller.moveView = MoveView::MoveSouth;
		}
		break;
		case sf::Event::MouseButtonReleased:
		{
			if (event.mouseButton.button == sf::Mouse::Left) {
				if (minimap.rect.contains(sf::Vector2f(mousePos))) {
					interface.clearSelected();
				} else {
					if (controller.action == Action::Selecting) {
						controller.selectionEnd = gamePos;
						sf::FloatRect selectRect(controller.selectionStart, controller.selectionEnd - controller.selectionStart);
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
									EntityID entity = this->map->objs.get(x, y);
									if (entity && this->vault->registry.has<Unit>(entity) && !this->vault->registry.has<Building>(entity)) {
										interface.addSelected(entity);
									}
								}
							}
						}
						interface.clearSelected();
					}
				}
			}
		}
		break;
		case sf::Event::MouseButtonPressed:
		{
			if (event.mouseButton.button == sf::Mouse::Left) {
				controller.selectedDebugObj = 0;
				// left click on minimap
				if (this->minimap.rect.contains(sf::Vector2f(mousePos))) {
					sf::Vector2f mPos((float)(mousePos.x - this->minimap.rect.left) / (this->minimap.size / this->map->width), (float)(mousePos.y - this->minimap.rect.top) / (this->minimap.size / this->map->width));
#ifdef GAME_ENGINE_DEBUG
					std::cout << "GameEngine: minimap clicked " << mPos.x << "x" << mPos.y << std::endl;
#endif
					this->centerMapView(sf::Vector2i(mPos));
					interface.clearSelected();

				} else {
					if (controller.action == Action::Building)
					{
						if (this->canBuild(controller.currentPlayer, controller.currentBuild).size() == 0) {
							if (!this->vault->factory.placeBuilding(this->vault->registry, controller.currentBuild)) {
								Player &player = this->vault->registry.get<Player>(controller.currentPlayer);
								player.rootConstruction = 0;
							}

							controller.action = Action::None;
							controller.currentBuild = 0;
						}
					} else {
						controller.action = Action::Selecting;
						controller.selectionStart = gamePos;

						controller.selectedObjs.clear();
						if (this->map->bound(gameMapPos.x, gameMapPos.y)) {
							EntityID entity = this->map->objs.get(gameMapPos.x, gameMapPos.y);

							// select building only
							if (entity && this->vault->registry.has<Building>(entity) && !this->vault->registry.has<Unit>(entity)) {
								interface.addSelected(entity);
							}

							controller.selectedDebugObj = entity;
							if (!controller.selectedDebugObj)
								controller.selectedDebugObj = this->map->resources.get(gameMapPos.x, gameMapPos.y);
							if (!controller.selectedDebugObj)
								controller.selectedDebugObj = this->map->decors.get(gameMapPos.x, gameMapPos.y);
						}
					}
				}
			}

			if (event.mouseButton.button == sf::Mouse::Right) {
				if (controller.action == Action::Building)
				{
					this->vault->registry.remove<Tile>(controller.currentBuild);
					controller.currentBuild = 0;
					controller.action = Action::None;
					this->markUpdateLayer = true;
				} else {
					// right click on minimap
					if (this->minimap.rect.contains(sf::Vector2f(mousePos))) {
						sf::Vector2f mPos((float)(mousePos.x - this->minimap.rect.left) / (this->minimap.size / this->map->width), (float)(mousePos.y - this->minimap.rect.top) / (this->minimap.size / this->map->width));
						interface.orderSelected(mPos);
					} else {
						interface.orderSelected(gameMapPos);
					}
				}

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
		interface.clearSelected();
	}

}