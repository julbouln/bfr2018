#pragma once
#include "Helpers.hpp"
#include "GameSystem.hpp"

class MapLayersSystem : public GameSystem {
	std::map<int, int> terrainTransitionsMapping;
	std::map<int, int> fogTransitionsMapping;

//	std::vector<EntityID> debugTransitions;

public:
	void update(float dt) override;
	void updateObjsLayer(float dt);
	void updateFog(float dt);

	void init() override;

private:
	void updateLayer(float dt);
	void updatePlayersFog(float dt);

	void updatePlayerFogLayer(EntityID playerEnt, float dt);
	// spectator FOG concat all other players fog
	void updateSpectatorFog(EntityID playerEnt, float dt);

// Terrains/Transitions
	void updateAllTransitions();
	void initTransitions();

// https://gamedevelopment.tutsplus.com/tutorials/how-to-use-tile-bitmasking-to-auto-tile-your-level-layouts--cms-25673
	int transitionBitmask(Layer<int> & layer, EntityID ent, int x, int y);
	int voidTransitionBitmask(Layer<int> & layer, EntityID ent, int x, int y);
	int pairTransitionBitmask(Layer<int> & layer, EntityID srcEnt, EntityID dstEnt, int x, int y);
	int updateTransition(int bitmask, Layer<int> & outLayer, EntityID ent, std::map<int, int> &mapping, int x, int y);

	// terrains transition
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