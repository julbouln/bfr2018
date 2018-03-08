#pragma once

#include "GameSystem.hpp"

class FxSystem : public GameSystem {
	sf::RenderTexture renderTexture;

	std::queue<EffectCreate> fxCreateQueue;
	std::queue<EffectDestroy> fxDestroyQueue;
public:
	void init();
	void update(float dt);
	void draw(sf::RenderWindow &window, sf::IntRect clip, float dt);

// signals
	void receive(const EffectCreate &event);
	void receive(const EffectDestroy &event);

	void clear();

private:
	void createEffects(float dt);
	void createEffect(const EffectCreate &event);
	void destroyEffects(float dt);
};
