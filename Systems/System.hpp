#pragma once

#include "Components/Components.hpp"
#include "GameVault.hpp"

class System {
public:
	GameVault *vault;

	void setVault(GameVault *vault) {
		this->vault = vault;
	}

	float approxDistance(sf::Vector2i p1, sf::Vector2i p2) {
		double dx = abs(p1.x - p2.x);
		double dy = abs(p1.y - p2.y);
		return 0.394 * (dx + dy) + 0.554 * std::max(dx, dy);
	}

	int getDirection(sf::Vector2i src, sf::Vector2i dst) {
		sf::Vector2i diff = dst - src;
		if (diff.x <= -1) {

			if (diff.y <= -1)
				return NorthWest;

			if (diff.y == 0)
				return West;

			if (diff.y >= 1)
				return SouthWest;
		}

		if (diff.x == 0) {
			if (diff.y <= -1)
				return North;

			if (diff.y == 0)
				return North;

			if (diff.y >= 1)
				return South;
		}

		if (diff.x >= 1) {
			if (diff.y <= -1)
				return NorthEast;

			if (diff.y == 0)
				return East;

			if (diff.y >= 1)
				return SouthEast;
		}

	}

};
