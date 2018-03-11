#pragma once
#include "Helpers.hpp"
#include "GameSystem.hpp"

class MapLayersSystem : public GameSystem {
	std::map<std::string, std::vector<EntityID>> tiles;

	std::vector<EntityID> dirtTransitions;
	std::vector<EntityID> waterTransitions;
	std::vector<EntityID> sandTransitions;
	std::vector<EntityID> concreteTransitions;

	std::map<int, int> terrainTransitionsMapping;
	std::vector<EntityID> fogTransitions;
	std::map<int, int> fogTransitionsMapping;

//	std::vector<EntityID> debugTransitions;

	std::map<EntityID, EntityID> altTerrains;

public:
	void update(float dt);

	void updateObjsLayer(float dt);
	void updatePlayerFogLayer(EntityID playerEnt, float dt);
	// spectator FOG concat all other players fog
	void updateSpectatorFog(EntityID playerEnt, float dt);

	void updateAllTransitions();

	void initTransitions();

	void init() override;

private:
	void updateLayer(float dt);
	void updatePlayersFog(float dt);

// Terrains/Transitions

// https://gamedevelopment.tutsplus.com/tutorials/how-to-use-tile-bitmasking-to-auto-tile-your-level-layouts--cms-25673
	int transitionBitmask(Layer<int> & layer, EntityID ent, int x, int y);
	int voidTransitionBitmask(Layer<int> & layer, EntityID ent, int x, int y);
	int pairTransitionBitmask(Layer<int> & layer, EntityID srcEnt, EntityID dstEnt, int x, int y);
	int updateTransition(int bitmask, Layer<int> & outLayer, EntityID ent, std::vector<EntityID> &transitions, std::map<int, int> &mapping, int x, int y);
	void updateGrassConcreteTransition(int x, int y);
	void updateSandWaterTransition(int x, int y);
	void updateGrassSandTransition(int x, int y);
	void updateConcreteSandTransition(int x, int y);
	void updateDirtTransition(int x, int y);
	void updateTransitions(float dt);
	// FOG transition
	void updateFogUnvisitedTransition(int x, int y);
	void updateFogHiddenTransition(int x, int y);

};