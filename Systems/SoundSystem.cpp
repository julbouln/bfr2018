#include "SoundSystem.hpp"

#define MAX_SOUNDS 32

void SoundSystem::init() {
	this->vault->dispatcher.connect<SoundPlay>(this);
}

void SoundSystem::receive(const SoundPlay &event) {
	this->sounds.push(event);
}

void SoundSystem::update(float dt) {
	float pitch = 0.033 / dt;

#ifdef SOUND_SYSTEM_DEBUG
	if (this->sounds.size() > 0)
		std::cout << "SoundSystem: will play " << this->sounds.size() << std::endl;
#endif

	while (this->sounds.size() > 0) {
		SoundPlay sndp = this->sounds.top();
		if (sndp.name != "") {
			if (playing.size() < MAX_SOUNDS) {
				sf::Sound sound;
#ifdef SOUND_SYSTEM_DEBUG
				std::cout << "SoundSystem: play " << sndp.priority << " " << sndp.name << " at " << sndp.pos.x << "x" << sndp.pos.y << std::endl;
#endif
				sound.setBuffer(this->vault->factory.getSndBuf(sndp.name));

				sound.setRelativeToListener(sndp.relative);
				sound.setPosition(sndp.pos.x, 0.f, sndp.pos.y);
				sound.setMinDistance(16.f);
				sound.setPitch(pitch);

				this->playing.push_back(sound);
				this->playing.back().play();
			} else {
#ifdef SOUND_SYSTEM_DEBUG
				std::cout << "SoundSystem: too many sound playing, drop " << sndp.name << std::endl;
#endif
			}

		}

		this->sounds.pop();
	}

	playing.remove_if([](sf::Sound & sound) { return (sound.getStatus() == sf::Sound::Status::Stopped); });

#ifdef SOUND_SYSTEM_DEBUG
//		if (playing.size() > 0)
//			std::cout << "SoundSystem: still playing " << playing.size() << " sounds" << std::endl;
//		else
//			std::cout << "SoundSystem: done playing sounds" << std::endl;

#endif
}
