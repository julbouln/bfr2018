#pragma once

#include "Components/Components.hpp"
#include "GameVault.hpp"

class System {
public:
	GameVault *vault;

	void setVault(GameVault *vault) {
		this->vault = vault;
	}
};
