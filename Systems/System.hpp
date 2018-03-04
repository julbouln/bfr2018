#pragma once

#include "Components/Components.hpp"
#include "GameVault.hpp"

class System {
public:
	GameVault *vault;

	void setVault(GameVault *vault) {
		this->vault = vault;
	}

	int getDirection(sf::Vector2i src, sf::Vector2i dst) {
		sf::Vector2i diff = dst - src;
		return this->getDirection(diff);
	}
	
	int getDirection(sf::Vector2i diff) {
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
