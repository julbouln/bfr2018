#include "TileAnimSystem.hpp"

void TileAnimSystem::init() {
	this->vault->dispatcher.connect<StateChanged>(this);
}

void TileAnimSystem::receive(const StateChanged &event) {
	EntityID entity = event.entity;
	if (this->vault->registry.has<AnimatedSpritesheet>(entity)) {
		AnimatedSpritesheet &spritesheet = this->vault->registry.get<AnimatedSpritesheet>(entity);
		AnimatedSpriteView &view = spritesheet.states[event.newState][event.view];
		Timer timer(event.newState,view.duration * view.frames.size(), view.loop);

		this->vault->registry.accomodate<Timer>(entity, timer);
		
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
	auto view = this->vault->registry.persistent<Tile, AnimatedSpritesheet, Timer>();
	for (EntityID entity : view) {
		Tile &tile = view.get<Tile>(entity);
		AnimatedSpritesheet &spritesheet = view.get<AnimatedSpritesheet>(entity);
		Timer &timer = view.get<Timer>(entity);
		if (spritesheet.states.count(tile.state) > 0) {
			AnimatedSpriteView &animView = spritesheet.states[tile.state][tile.view];
			int frames = animView.frames.size();

			if (int((timer.t + dt) / (timer.duration / frames)) > int(timer.t / (timer.duration / frames)))
			{
				/* Calculate the frame number */
				int frame = int((timer.t + dt) / (timer.duration / frames));

				/* Adjust for looping */
				if (timer.loop)
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


		}
	}
}