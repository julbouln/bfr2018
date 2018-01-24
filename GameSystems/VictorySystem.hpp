#pragma once

#include "GameSystem.hpp"

class VictorySystem : public GameSystem {
public:
	void updateStats(float dt) {
		auto playerView = this->vault->registry.view<Player>();

		for (EntityID entity : playerView) {
			Player &player = playerView.get(entity);
//			std::cout << "Player "<<entity<< " "<<player.team<<" kills "<<player.kills.size() << std::endl;

			player.butchery += pow(player.kills.size(), 2);

			switch (player.kills.size()) {
			case 0:
				break;
			case 1:
				break;
			case 2:
#ifdef VICTORY_DEBUG
				std::cout << "VictorySystem: COMBO " << player.team <<  std::endl;
#endif
				player.stats["combo"] = player.stats["combo"] + 1;
				break;
			case 3:
#ifdef VICTORY_DEBUG
				std::cout << "VictorySystem: SERIAL-KILLER " << player.team <<  std::endl;
#endif
				player.stats["killer"] = player.stats["killer"] + 1;
				break;
			case 4:
#ifdef VICTORY_DEBUG
				std::cout << "VictorySystem: MEGAKILL " << player.team << std::endl;
#endif
				player.stats["megakill"] = player.stats["megakill"] + 1;
				break;
			case 5:
#ifdef VICTORY_DEBUG
				std::cout << "VictorySystem: BARBARIAN " << player.team << std::endl;
#endif
				player.stats["barbarian"] = player.stats["barbarian"] + 1;
				break;
			default:
#ifdef VICTORY_DEBUG
				std::cout << "VictorySystem: BUTCHERY " << player.team << std::endl;
#endif
				player.stats["butchery"] = player.stats["butchery"] + 1;
				break;
			}

			player.stats["kills"] = player.stats["kills"] + player.kills.size();
		}
	}

	float resourcesVictory() {
		return (float)(this->map->width * this->map->height) / 2.0;
	}

	float butcheryVictory() {
		return (float)(this->map->width * this->map->height) / 2.0;
	}

	bool checkVictoryConditions(EntityID playerEnt) {
		Player &player = this->vault->registry.get<Player>(playerEnt);
		if ((float)player.resources / this->resourcesVictory() >= 1.0) {
			return true;
		}
		if ((float)player.butchery / this->butcheryVictory() >= 1.0) {
			return true;
		}

		int ennemyObjs = 0;

		auto view = this->vault->registry.view<Player>();
		for (EntityID entity : view) {
			if (entity != playerEnt) {
				Player &otherPlayer = view.get(entity);
				if (otherPlayer.team != player.team) {
					for (auto o : otherPlayer.objsByType) {
						ennemyObjs += o.second.size();
					}
				}

			}
		}

		if (ennemyObjs == 0)
			return true;

		return false;
	}

};