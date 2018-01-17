#pragma once

#include "GameSystem.hpp"

class TileAnimSystem : public GameSystem {
public:
	void update(float dt) {
		auto view = this->vault->registry.view<Tile>();
		for (EntityID entity : view) {
			Tile &tile = view.get(entity);

			AnimationHandler &currentAnim = tile.animHandlers[tile.state];

			/* Change the sprite to reflect the tile variant */
			currentAnim.changeAnim(tile.direction);

			/* Update the animation */
			currentAnim.update(dt);

			/* Update the sprite */
			tile.sprite.setTextureRect(currentAnim.bounds);
		}
	}
};