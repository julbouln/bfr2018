#pragma once

#include "GameSystem.hpp"

class FxSystem : public GameSystem {
public:
	void update(sf::Time &elapsed) {

		auto view = this->vault->registry.view<ParticleEffect>();
		for (EntityID entity : view) {
			ParticleEffect &effect = view.get(entity);
			effect.currentTime += elapsed.asSeconds();
			
			if(effect.currentTime >= effect.lifetime) {
				delete effect.particleSystem;
				this->vault->factory.destroyEntity(this->vault->registry,entity);
			} else {
				effect.particleSystem->update(elapsed);
			}
		}
	}

	void draw(sf::RenderWindow &window, float dt) {

		auto view = this->vault->registry.view<ParticleEffect>();
		for (EntityID entity : view) {
			ParticleEffect &effect = view.get(entity);
			effect.particleSystem->render(window);
		}
	}
};
