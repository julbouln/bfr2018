#pragma once

#include "Entity.hpp"
#include "EntityFactory.hpp"
#include "third_party/entt/signal/dispatcher.hpp"

struct GameVault {
	entt::Registry<EntityID> registry;
	EntityFactory factory;
	entt::UnmanagedDispatcher dispatcher;
};
