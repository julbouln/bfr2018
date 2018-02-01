#pragma once

#include "GameSystem.hpp"

class TileAnimSystem : public GameSystem {
public:
	void update(float dt) {
		float realDt = 0.033 / dt * 0.033;
		auto view = this->vault->registry.view<Tile>();
		for (EntityID entity : view) {
			Tile &tile = view.get(entity);

			AnimationHandler &currentAnim = tile.animHandlers[tile.state];

			/* Change the sprite to reflect the tile variant */
			currentAnim.changeColumn(tile.direction);

			/* Update the animation */
			currentAnim.update(realDt);

			/* Update the sprite */
			tile.sprite.setTextureRect(currentAnim.bounds);
		}
#if 0
		updateStaticSpritesheets(dt);
		updateAnimatedSpritesheets(dt);
#endif
	}

	void updateStaticSpritesheets(float dt) {
		auto view = this->vault->registry.persistent<Tile, StaticSpritesheet>();
		for (EntityID entity : view) {
			Tile &tile = view.get<Tile>(entity);
			StaticSpritesheet &spritesheet = view.get<StaticSpritesheet>(entity);
			SpriteView &staticView = spritesheet.states[tile.state][tile.view];
//			std::cout << "static sprite "<<tile.state<<" "<<tile.view << " "<< spritesheet.states.count(tile.state) << " " << spritesheet.states[tile.state].size() << " " << (void*)&staticView << std::endl;
			sf::Vector2i pos(staticView.currentPosition.x * tile.psize.x, staticView.currentPosition.y * tile.psize.y);
			sf::IntRect boundingRect(pos, sf::Vector2i(tile.psize));
			tile.sprite.setTextureRect(boundingRect);
		}
	}

	void updateAnimatedSpritesheets(float dt) {
		auto view = this->vault->registry.persistent<Tile, AnimatedSpritesheet>();
		for (EntityID entity : view) {
			Tile &tile = view.get<Tile>(entity);
			AnimatedSpritesheet &spritesheet = view.get<AnimatedSpritesheet>(entity);
			AnimatedSpriteView &animView = spritesheet.states[tile.state][tile.view];

			if (int((animView.t + dt) / animView.duration) > int(animView.t / animView.duration))
			{
				/* Calculate the frame number */
				int frame = int((animView.t + dt) / animView.duration);

				/* Adjust for looping */
				if (animView.loop)
					frame %= animView.frames.size();

				if (frame != animView.currentFrame) {
					animView.frameChangeCallback(frame);
				}

				animView.currentFrame = frame;

				if (animView.currentFrame < animView.frames.size()) {
					sf::Vector2i currentPosition = animView.frames[animView.currentFrame];
					sf::Vector2i pos(currentPosition.x * tile.psize.x, currentPosition.y * tile.psize.y);
					sf::IntRect boundingRect(pos, sf::Vector2i(tile.psize));
					tile.sprite.setTextureRect(boundingRect);
				}
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
};
