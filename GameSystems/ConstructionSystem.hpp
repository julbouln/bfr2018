#pragma once

#include "GameSystem.hpp"

class ConstructionSystem : public GameSystem {
public:
	void update(float dt) {
		float realDt = 0.1 / dt * 0.1;
		auto view = this->vault->registry.view<Building>();
		for (EntityID entity : view) {
			Building &building = view.get(entity);
			if (building.buildTime > 0) {
//				std::cout << "update construction " << dt << " " << entity << " " << building.buildTime << std::endl;
				building.buildTime -= realDt;
				if (building.buildTime <= 0)
					building.buildTime = 0.0;
			}
		}
	}
};
