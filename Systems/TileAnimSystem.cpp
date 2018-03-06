#include "TileAnimSystem.hpp"

void TileAnimSystem::init() {
	this->vault->dispatcher.connect<StateChanged>(this);
}

void TileAnimSystem::receive(const StateChanged &event) {
	EntityID entity = event.entity;
	if (this->vault->registry.has<AnimatedSpritesheet>(entity)) {
		AnimatedSpritesheet &spritesheet = this->vault->registry.get<AnimatedSpritesheet>(entity);

		AnimatedSpriteView &view = spritesheet.states[event.state][event.view];
		view.l = 0;
		view.t = 0.0;
		view.currentFrame = 0;
	}
}

void TileAnimSystem::update(float dt) {
	float gameDt = 0.033 / dt * 0.033;
	updateStaticSpritesheets(gameDt);
	updateAnimatedSpritesheets(gameDt);
}

void TileAnimSystem::updateStaticSpritesheets(float dt) {
	auto view = this->vault->registry.persistent<Tile, StaticSpritesheet>();
	for (EntityID entity : view) {
		Tile &tile = view.get<Tile>(entity);
		StaticSpritesheet &spritesheet = view.get<StaticSpritesheet>(entity);
		if (spritesheet.states.count(tile.state) > 0) {
			SpriteView &staticView = spritesheet.states[tile.state][tile.view];
			sf::Vector2i pos(staticView.currentPosition.x * tile.psize.x, staticView.currentPosition.y * tile.psize.y);
			sf::IntRect boundingRect(pos, sf::Vector2i(tile.psize));
			tile.sprite.setTextureRect(boundingRect);
		}
	}
}

void TileAnimSystem::updateAnimatedSpritesheets(float dt) {
	auto view = this->vault->registry.persistent<Tile, AnimatedSpritesheet>();
	for (EntityID entity : view) {
		Tile &tile = view.get<Tile>(entity);
		AnimatedSpritesheet &spritesheet = view.get<AnimatedSpritesheet>(entity);
		if (spritesheet.states.count(tile.state) > 0) {
			AnimatedSpriteView &animView = spritesheet.states[tile.state][tile.view];

			if (int((animView.t + dt) / animView.duration) > int(animView.t / animView.duration))
			{
				/* Calculate the frame number */
				int frame = int((animView.t + dt) / animView.duration);

				/* Adjust for looping */
				if (animView.loop)
					frame %= animView.frames.size();

				if (frame != animView.currentFrame) {
					this->vault->dispatcher.trigger<AnimationFrameChanged>(entity, tile.state, frame);
				}

				animView.currentFrame = frame;
			}

			if (animView.currentFrame < animView.frames.size()) {
				sf::Vector2i currentPosition = animView.frames[animView.currentFrame];
				sf::Vector2i pos(currentPosition.x * tile.psize.x, currentPosition.y * tile.psize.y);
				sf::IntRect boundingRect(pos, sf::Vector2i(tile.psize));
				tile.sprite.setTextureRect(boundingRect);
			}

			// increment the time elapsed
			animView.t += dt;

			if (animView.t > animView.duration * animView.frames.size()) {
				// reset time and increment loop count if loop
				if (animView.loop) {
					animView.t = 0.0f;
					animView.l++;
				} else {
					animView.l = 1;
				}
			}
		}
	}
}