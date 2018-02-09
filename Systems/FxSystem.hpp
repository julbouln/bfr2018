#pragma once

#include "GameSystem.hpp"

class FxSystem : public GameSystem {
public:
	void update(sf::Time &elapsed) {
		auto view = this->vault->registry.view<ParticleEffect>();
		for (EntityID entity : view) {
			ParticleEffect &effect = view.get(entity);
			effect.currentTime += elapsed.asSeconds();

			if (effect.currentTime >= effect.lifetime || (!effect.continuous && effect.particleSystem->countAlive() == 0)) {
				delete effect.particleSystem;
				this->vault->factory.destroyEntity(this->vault->registry, entity);
			} else {
				effect.particleSystem->update(elapsed);
			}
		}
	}

	void clear() {
		auto view = this->vault->registry.view<ParticleEffect>();
		for (EntityID entity : view) {
			ParticleEffect &effect = view.get(entity);
			delete effect.particleSystem;
			this->vault->factory.destroyEntity(this->vault->registry, entity);
		}
	}

	void draw(sf::RenderWindow &window, sf::IntRect clip, float dt) {
		auto view = this->vault->registry.view<ParticleEffect>();
		for (EntityID entity : view) {
			ParticleEffect &effect = view.get(entity);
			if (effect.spawner->center.x / 32 > clip.left && effect.spawner->center.y / 32 > clip.top &&
			        effect.spawner->center.x / 32 < clip.left + clip.width && effect.spawner->center.y / 32 < clip.top + clip.height)
				effect.particleSystem->render(window);
		}
	}
};
