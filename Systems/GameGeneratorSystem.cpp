#include "GameGeneratorSystem.hpp"

void GameGeneratorSystem::generateMap(unsigned int width, unsigned int height) {
	this->map->setSize(width, height);

	float random_w = ((float) rand()) / (float) RAND_MAX;
	float random_h = ((float) rand()) / (float) RAND_MAX;

	SimplexNoise simpl(width / 64.0, height / 64.0, 2.0, 0.5);

	for (float y = 0; y < height; y++) {
		for (float x = 0; x < width; x++) {
			float res = (simpl.fractal(64, x / (width) + random_w * width, y / (height) + random_h * height));

			EntityID t;

			t = Dirt + (rand() % ALT_TILES);
			this->map->terrainsForTransitions.set(x, y, Dirt);

			// sand
			if (res < -0.2) {
				t = Sand + (rand() % ALT_TILES);
				this->map->staticBuildable.set(x, y, t);
				this->map->terrainsForTransitions.set(x, y, Sand);
			}

			// water
			if (res < -0.4) {
				t = Water + (rand() % ALT_TILES);

				this->map->staticBuildable.set(x, y, t);
				this->map->staticPathfinding.set(x, y, t);
				this->map->terrainsForTransitions.set(x, y, Water);
			}

			this->map->terrains[Terrain].set(x, y, t);

			// add some random ressources
			if (res > 0.6 && res < 0.61) {
				float rnd = ((float) rand()) / (float) RAND_MAX;
				if (rnd > 0.5) {
					this->vault->factory.plantResource(this->vault->registry, "nature", x, y);
				} else {
					this->vault->factory.plantResource(this->vault->registry, "pollution", x, y);
				}
			}
		}
	}

	for (auto pair : this->vault->factory.decorGenerator) {
		for (int i = 0; i < (this->map->width * this->map->height) / pair.second; i++) {
			int rx = rand() % this->map->width;
			int ry = rand() % this->map->height;
			if (this->map->terrainsForTransitions.get(rx, ry) != Water) {
				this->vault->factory.createDecor(this->vault->registry, pair.first, rx, ry);
			}
		}
	}

	// set decor layer
	auto decorView = this->vault->registry.persistent<Tile, Decor>();

	for (EntityID entity : decorView) {
		Tile &tile = decorView.get<Tile>(entity);
		Decor &decor = decorView.get<Decor>(entity);

		for (sf::Vector2i const &p : this->tileSurface(tile)) {
			this->map->decors.set(p.x, p.y, entity);
			if (decor.blocking) {
				this->map->staticBuildable.set(p.x, p.y, entity);
				this->map->staticPathfinding.set(p.x, p.y, entity);
			}
		}
	}
}

EntityID GameGeneratorSystem::generate(int mapWidth, int mapHeight, std::string playerTeam) {
	EntityID currentPlayer;

	this->generateMap(mapWidth, mapHeight);

	std::vector<int> colorIndices = this->generateColorIndices();
	std::vector<sf::Vector2i> initialPositions = this->generateInitialPositions();

	if (playerTeam == "rebel") {
		currentPlayer = this->vault->factory.createPlayer(this->vault->registry, "rebel", false);
		this->vault->factory.createPlayer(this->vault->registry, "neonaz", true);
	} else if (playerTeam == "neonaz") {
		currentPlayer = this->vault->factory.createPlayer(this->vault->registry, "neonaz", false);
		this->vault->factory.createPlayer(this->vault->registry, "rebel", true);
	} else {
		currentPlayer = this->vault->factory.createPlayer(this->vault->registry, "neutral", false);
		this->vault->factory.createPlayer(this->vault->registry, "rebel", true);
		this->vault->factory.createPlayer(this->vault->registry, "neonaz", true);

		this->vault->factory.createPlayer(this->vault->registry, "rebel", true);
		this->vault->factory.createPlayer(this->vault->registry, "neonaz", true);
	}

	auto view = this->vault->registry.view<Player>();
	for (EntityID entity : view) {
		Player &player = view.get(entity);
		sf::Color refCol = sf::Color(3, 255, 205);

		player.colorIdx = colorIndices.back();
		player.color = this->vault->factory.getPlayerColor(refCol, player.colorIdx);
		colorIndices.pop_back();

		if (player.team == "rebel")
		{
			player.initialPos = initialPositions.back();
			initialPositions.pop_back();

			this->vault->factory.createUnit(this->vault->registry, entity, "zork", player.initialPos.x, player.initialPos.y);

/*
			for (int x = 0; x < 3; x++) {
				for (int y = 0; y < 3; y++) {
					this->vault->factory.createUnit(this->vault->registry, entity, "zork", player.initialPos.x + x, player.initialPos.y + y);

				}

			}
*/			
		} else if (player.team == "neonaz") {
			player.initialPos = initialPositions.back();
			initialPositions.pop_back();

			this->vault->factory.createUnit(this->vault->registry, entity, "brad_lab", player.initialPos.x, player.initialPos.y);
		}

		player.fog.width = this->map->width;
		player.fog.height = this->map->height;
		player.fog.fill();
	}

	return currentPlayer;
}

std::vector<int> GameGeneratorSystem::generateColorIndices() {
	std::vector<int> colorIndices;
	for (int i = 0; i < 12; i++ ) {
		colorIndices.push_back(i);
	}
	std::random_shuffle ( colorIndices.begin(), colorIndices.end() );
	return colorIndices;
}

sf::Vector2i GameGeneratorSystem::getInitialPosition(sf::Vector2i pos) {
	return this->firstAvailablePosition(pos, 1, 16);
}

std::vector<sf::Vector2i> GameGeneratorSystem::generateInitialPositions() {
	std::vector<sf::Vector2i> initialPositions;
	initialPositions.push_back(this->getInitialPosition(sf::Vector2i(10, 10)));
	initialPositions.push_back(this->getInitialPosition(sf::Vector2i(10, this->map->width - 10)));
	initialPositions.push_back(this->getInitialPosition(sf::Vector2i(this->map->height - 10, 10)));
	initialPositions.push_back(this->getInitialPosition(sf::Vector2i(this->map->width - 10, this->map->height - 10)));
	std::random_shuffle ( initialPositions.begin(), initialPositions.end() );
	return initialPositions;
}
