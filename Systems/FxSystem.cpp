#include "Events.hpp"
#include "FxSystem.hpp"

#define MAX_EFFECTS_QUEUE 32

void FxSystem::init() {
	renderTexture.create(this->screenWidth, this->screenHeight);

	this->vault->dispatcher.connect<EffectCreate>(this);
	this->vault->dispatcher.connect<EffectDestroy>(this);
}

void FxSystem::receive(const EffectCreate &event) {
	fxCreateQueue.push(event);
}

void FxSystem::receive(const EffectDestroy &event) {
	fxDestroyQueue.push(event);
}

void FxSystem::createEffect(const EffectCreate &event) {
#ifdef PARTICLES_ENABLE
	std::string name = event.name;
	ParticleEffectOptions options = event.options;
	sf::Vector2f ppos = event.ppos;

	if (event.emitterEnt) {
		EntityID emitter = event.emitterEnt;

		if (this->vault->registry.valid(emitter) && this->vault->registry.has<Effects>(emitter)) {
			Effects &effects = this->vault->registry.get<Effects>(emitter);
			if (effects.effects.count(name) > 0) {
				EntityID entity = this->vault->factory.createParticleEffect(this->vault->registry, effects.effects[name], options);
				ParticleEffect &effect = this->vault->registry.get<ParticleEffect>(entity);
				effect.spawner->center = ppos;
				if (!effect.continuous)
					effect.particleSystem->emitParticles(effect.particles);

				this->vault->dispatcher.trigger<EffectCreated>(event.name, entity);
			}
		}
	} else {
		EntityID entity = this->vault->factory.createParticleEffect(this->vault->registry, name, options);
		ParticleEffect &effect = this->vault->registry.get<ParticleEffect>(entity);

		effect.spawner->center = ppos;
		if (!effect.continuous)
			effect.particleSystem->emitParticles(effect.particles);

		this->vault->dispatcher.trigger<EffectCreated>(event.name, entity);
	}
#endif
}

void FxSystem::createEffects(float dt) {
	int created = 0;
//	std::cout << "FxSystem: " << fxCreateQueue.size() << " effect creation awaiting" << std::endl;
	while (!fxCreateQueue.empty() && created < MAX_EFFECTS_QUEUE)
	{
		EffectCreate &effect = fxCreateQueue.front();
		this->createEffect(effect);
		fxCreateQueue.pop();
		created++;
	}
}

void FxSystem::destroyEffects(float dt) {
	while (!fxDestroyQueue.empty()) {
		EffectDestroy &effectEv = fxDestroyQueue.front();
		if (this->vault->registry.valid(effectEv.entity)) {
			ParticleEffect &effect = this->vault->registry.get<ParticleEffect>(effectEv.entity);
			delete effect.particleSystem;
			this->vault->factory.destroyEntity(this->vault->registry, effectEv.entity);
		}
		fxDestroyQueue.pop();
	}

}

void FxSystem::update(float dt) {
	float gameDt = 0.033 / dt * 0.033;

	this->createEffects(gameDt);
	this->destroyEffects(gameDt);

	auto view = this->vault->registry.view<ParticleEffect>();
	for (EntityID entity : view) {
		ParticleEffect &effect = view.get(entity);
		effect.currentTime += gameDt;

		if (effect.currentTime >= effect.lifetime || (!effect.continuous && effect.particleSystem->countAlive() == 0)) {
//			effect.effectEndCallback();
			this->vault->dispatcher.trigger<EffectEnded>(entity);

			ParticleEffectOptions nOptions;
			this->vault->dispatcher.trigger<EffectCreate>("next", entity, effect.destpos, nOptions);
			this->vault->dispatcher.trigger<EffectDestroy>(entity);
		} else {
			effect.particleSystem->update(dt);
		}
	}
}

void FxSystem::clear() {
	auto view = this->vault->registry.view<ParticleEffect>();
	for (EntityID entity : view) {
		ParticleEffect &effect = view.get(entity);
		delete effect.particleSystem;
		this->vault->factory.destroyEntity(this->vault->registry, entity);
	}
}

void FxSystem::draw(sf::RenderWindow &window, sf::IntRect clip, float dt) {
	auto view = this->vault->registry.view<ParticleEffect>();
	for (EntityID entity : view) {
		ParticleEffect &effect = view.get(entity);
		if (effect.alwaysVisible || (effect.spawner->center.x / 32 > clip.left && effect.spawner->center.y / 32 > clip.top &&
		                             effect.spawner->center.x / 32 < clip.left + clip.width && effect.spawner->center.y / 32 < clip.top + clip.height))
			effect.particleSystem->render(window, &renderTexture);
	}
}
