#include "ConstructionSystem.hpp"

void ConstructionSystem::update(float dt) {
	float gameDt = 0.1 / dt * 0.1;
	auto view = this->vault->registry.view<Building>();
	for (EntityID entity : view) {
		Building &building = view.get(entity);
		if (building.buildTime > 0) {
//				std::cout << "update construction " << dt << " " << entity << " " << building.buildTime << std::endl;
			building.buildTime -= gameDt;
			if (building.buildTime <= 0)
				building.buildTime = 0.0;
		}
	}
}

