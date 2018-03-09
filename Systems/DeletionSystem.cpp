#include "DeletionSystem.hpp"
#include "Components/Components.hpp"

void DeletionSystem::init() {
	this->initCorpses();
	this->vault->dispatcher.connect<EntityDelete>(this);
}

void DeletionSystem::receive(const EntityDelete &event) {
	this->entities.push(event.entity);
}

void DeletionSystem::update(float dt) {
	while (!this->entities.empty()) {
		EntityID entity = this->entities.front();

		if (this->vault->registry.valid(entity)) { // check if already destroyed
			if (this->vault->registry.has<GameObject>(entity)) {
				Tile &tile = this->vault->registry.get<Tile>(entity);
				GameObject &obj = this->vault->registry.get<GameObject>(entity);

				if (this->vault->registry.has<Unit>(entity)) {
					Unit &unit = this->vault->registry.get<Unit>(entity);
					EntityID corpseEnt = corpses_and_ruins[obj.name + "_corpse_" + std::to_string(obj.player)];
//					std::cout << "GameEngine: set corpse " << obj.name + "_corpse" << " " << corpseEnt << " at " << tile.pos.x << " " << tile.pos.y << std::endl;
					this->map->corpses.set(tile.pos.x, tile.pos.y, corpseEnt);
				}
				if (this->vault->registry.has<Building>(entity)) {
					Building &building = this->vault->registry.get<Building>(entity);
					this->map->corpses.set(tile.pos.x, tile.pos.y, corpses_and_ruins[obj.team + "_ruin"]);

					if (building.construction) {
						// destroy currently building cons
						this->vault->factory.destroyEntity(this->vault->registry, building.construction);
					}
				}
			}
			this->vault->factory.destroyEntity(this->vault->registry, entity);
		}
		this->entities.pop();
	}

// clean destroyed targets
	auto view = this->vault->registry.view<Unit>();
	for (EntityID entity : view) {
		Unit &unit = view.get(entity);
		if (!this->vault->registry.valid(unit.targetEnt)) {
			unit.targetEnt = 0;
		}
	}

}

void DeletionSystem::initCorpse(std::string name, EntityID playerEnt) {
	Tile tile;
	this->vault->factory.parseTileFromXml(name, tile);

	tile.pos = sf::Vector2i(0, 0);
	tile.ppos = sf::Vector2f(tile.pos) * (float)32.0;
	tile.z = 0;

	tile.sprite.setTexture(this->vault->factory.getTex(name));
	tile.state = "die";
	tile.shader = false;
	this->vault->factory.setColorSwapShader(this->vault->registry, tile, playerEnt);

	tile.sprite.setTextureRect(sf::IntRect(0, ((this->vault->factory.getTex(name).getSize().y / tile.psize.y) - 1)*tile.psize.y, tile.psize.x, tile.psize.y)); // texture need to be updated

	tile.centerRect = this->vault->factory.getCenterRect(name);

	EntityID corpseEnt = this->vault->registry.create();
	this->vault->registry.assign<Tile>(corpseEnt, tile);
	corpses_and_ruins[name + "_corpse_" + std::to_string(playerEnt)] = corpseEnt;
}

void DeletionSystem::initRuin(std::string team, int i) {
	EntityID ruinEnt;
	int ruinHeight = this->vault->factory.getTex("ruin").getSize().y / 2;
	Tile ruinTile;
	ruinTile.pos = sf::Vector2i(0, 0);
	ruinTile.ppos = sf::Vector2f(ruinTile.pos) * (float)32.0;
	ruinTile.shader = false;
	ruinTile.psize = sf::Vector2f(this->vault->factory.getTex("ruin").getSize().x, ruinHeight);
	ruinTile.sprite.setTexture(this->vault->factory.getTex("ruin"));
	ruinTile.centerRect = this->vault->factory.getCenterRect("ruin");
	ruinTile.sprite.setTextureRect(sf::IntRect(0, i * ruinHeight, ruinTile.psize.x, ruinTile.psize.y)); // texture need to be updated

	ruinTile.state = "idle";
	ruinEnt = this->vault->registry.create();
	this->vault->registry.assign<Tile>(ruinEnt, ruinTile);
	corpses_and_ruins[team + "_ruin"] = ruinEnt;
}

// init corpses and ruins tiles, must be called after player creation
void DeletionSystem::initCorpses() {

	auto playerView = this->vault->registry.view<Player>();
	for (EntityID playerEnt : playerView) {
		Player &player = playerView.get(playerEnt);
		for (TechNode *node : this->vault->factory.getTechNodes(player.team)) {
			if (node->comp == TechComponent::Character) {
				this->initCorpse(node->type, playerEnt);
			}
		}
	}


	this->initRuin("rebel", 0);
	this->initRuin("neonaz", 0);

}