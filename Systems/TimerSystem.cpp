#include "TimerSystem.hpp"

void TimerSystem::init() {
}

void TimerSystem::update(float dt) {
	float gameDt = 0.033 / dt * 0.033;
	auto view = this->vault->registry.view<Timer>();

	// destroy timers with non-valid emitter or ended
	for (EntityID entity : view) {
		Timer &timer = view.get(entity);
		if((timer.emitterEntity && !this->vault->registry.valid(timer.emitterEntity)) || (timer.emitterEntity && timer.ended())) {
			this->vault->registry.destroy(entity);
		}
	}

	for (EntityID entity : view) {
		Timer &timer = view.get(entity);

		if (timer.t == 0.0 && timer.l == 0)
			this->vault->dispatcher.trigger<TimerStarted>(timer.name, entity);

		// increment the time elapsed
		timer.t += gameDt;

		if (timer.t > timer.duration) {
			// reset time and increment loop count if loop
			if (timer.loop) {
				this->vault->dispatcher.trigger<TimerLooped>(timer.name, entity, timer.l);
				timer.t = 0.0f;
				timer.l++;
			} else {
				this->vault->dispatcher.trigger<TimerEnded>(timer.name, entity);
				timer.l = 1;
			}
		}
	}


}
