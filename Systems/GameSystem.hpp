#pragma once

#include "Config.hpp"
#include "Events.hpp"

#include "GameVault.hpp"
#include "System.hpp"
#include "Map.hpp"

#include "third_party/entt/signal/dispatcher.hpp"

class GameSystem : public System {
public:
	Map *map;
	int screenWidth;
	int screenHeight;

	virtual void init();
	virtual void update(float dt);

	void setShared(GameVault *vault, Map *map, int screenWidth, int screenHeight);

	sf::Vector2f tileDrawPosition(Tile &tile) const;
	sf::Vector2i tilePosition(Tile &tile, sf::Vector2i p) const;
	std::vector<sf::Vector2i> tileSurface(Tile &tile) const;
	std::vector<sf::Vector2i> vectorSurfaceExtended(sf::Vector2i pos, int dist) const;
	std::vector<sf::Vector2i> tileSurfaceExtended(Tile &tile, int dist) const;
	std::vector<sf::Vector2i> tileAround(Tile &tile, int minDist, int maxDist) const;
	sf::Vector2i nearestTileAround(Tile &tile, Tile &destTile, int minDist, int maxDist) const;
	sf::Vector2i firstAvailablePosition(sf::Vector2i src, int minDist, int maxDist) const;

	EntityID ennemyAtPosition(EntityID playerEnt, int x, int y);
	bool targetInRange(Tile &tile, sf::Vector2i targetPos, int range, int maxRange);
	bool ennemyInRange(Tile &tile, Tile &destTile, int range, int maxRange);
	void addPlayerFrontPoint(EntityID playerEnt, EntityID ent, sf::Vector2i pos);
	std::vector<sf::Vector2i> canBuild(EntityID playerEnt, EntityID entity);

	bool canSpendResources(EntityID playerEnt, std::string type, int val);

	void spendResources(EntityID playerEnt, std::string type, int val);

	void changeState(EntityID entity, std::string state);

	void playRandomUnitSound(EntityID ent, std::string state);
	void playRandomUnitSound(GameObject & obj, Unit & unit, std::string state);

// action
	void seedResources(std::string type, EntityID entity);
	bool trainUnit(std::string type, EntityID playerEnt, EntityID entity );

	void groupAttackOrMove(EntityID playerEnt, std::vector<EntityID> &group, sf::Vector2i destpos);
	void groupAttackOrBomb(EntityID playerEnt, std::vector<EntityID> &group, sf::Vector2i destpos);
	void groupGoTo(std::vector<EntityID> &group, sf::Vector2i destpos, GroupFormation formation, int direction, bool playSound);

	void clearTarget(Unit & unit);
	void clearTarget(EntityID entity);
	void goTo(Unit & unit, sf::Vector2i destpos);
	void goTo(EntityID entity, sf::Vector2i destpos);
	void attack(Unit & unit, EntityID destEnt);
	void attack(EntityID entity, EntityID destEnt);

	void bomb(Unit & unit, sf::Vector2i destPos);
	void bomb(EntityID entity, sf::Vector2i destPos);

	inline float resourcesVictory() const {
		return (float)(this->map->width * this->map->height) / 2.0;
	}

	inline float butcheryVictory() const {
		return (float)(this->map->width * this->map->height) / 2.0;
	}

// scale for other resolutions than 800x600
	inline float scaleX() const {
		return this->screenWidth / 800.0;
	}
	inline float scaleY() const {
		return this->screenHeight / 600.0;
	}
};