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
	bool ennemyInRange(Tile &tile, Tile &destTile, int range, int maxRange);
	void addPlayerFrontPoint(EntityID playerEnt, EntityID ent, sf::Vector2i pos);
	std::vector<sf::Vector2i> canBuild(EntityID playerEnt, EntityID entity);

	bool canSpendResources(EntityID playerEnt, std::string type, int val);

	EntityID emitEffect(std::string name, sf::Vector2f ppos, ParticleEffectOptions options);

	EntityID emitEffect(std::string name, EntityID emitter, sf::Vector2f ppos, ParticleEffectOptions options);

	void spendResources(EntityID playerEnt, std::string type, int val);
	float buildTime(std::string type);
	float trainCost(std::string type);

	// get object initial life from XML
	float objTypeLife(std::string type);

	void changeState(EntityID entity, std::string state);

	void playSound(sf::Sound & snd, std::string name);

	void playRandomUnitSound(EntityID ent, std::string state);

	void playRandomUnitSound(GameObject & obj, Unit & unit, std::string state);

// action
	void seedResources(std::string type, EntityID entity);
	bool trainUnit(std::string type, EntityID playerEnt, EntityID entity );
	void sendGroup(std::vector<EntityID> group, sf::Vector2i destpos, GroupFormation formation, int direction, bool playSound);

	void clearTarget(Unit & unit);
	void clearTarget(EntityID entity);
	void goTo(Unit & unit, sf::Vector2i destpos);
	void goTo(EntityID entity, sf::Vector2i destpos);
	void stop(Unit &unit);
	void attack(Unit & unit, EntityID destEnt);
	void attack(EntityID entity, EntityID destEnt);

};