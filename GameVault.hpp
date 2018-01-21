#pragma once

#include "Entity.hpp"
#include "EntityFactory.hpp"

struct GameVault {
	entt::Registry<EntityID> registry;
	EntityFactory factory;
};
