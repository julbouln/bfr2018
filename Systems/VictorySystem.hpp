#pragma once

#include "GameSystem.hpp"

class VictorySystem : public GameSystem {
	bool scoreBonus;
	sf::Text scoreBonusText;
	sf::Sound scoreSound;

public:
	void init() {
		scoreBonusText.setFont(this->vault->factory.fntManager.getRef("samos"));
		scoreBonusText.setCharacterSize(48);
#if SFML_VERSION_MAJOR==2 && SFML_VERSION_MINOR > 3
		scoreBonusText.setFillColor(sf::Color::White);
#else
		// SFML 2.3
		scoreBonusText.setColor(sf::Color::White);
#endif
	}

	void updatePlayerBonus(EntityID entity) {
		Player &player = this->vault->registry.get<Player>(entity);
		switch (player.kills.size()) {
		case 0:
			this->scoreBonus = false;
			break;
		case 1:
			this->scoreBonus = false;
			// normal
			break;
		case 2:
			this->scoreBonus = true;
			this->scoreBonusText.setString("COMBO");
			this->map->sounds.push(SoundPlay{"combo", 5, true, sf::Vector2i{0, 0}});
			break;
		case 3:
			this->scoreBonus = true;
			this->scoreBonusText.setString("SERIAL KILLER");
			this->map->sounds.push(SoundPlay{"killer", 5, true, sf::Vector2i{0, 0}});
			break;
		case 4:
			this->scoreBonus = true;
			this->scoreBonusText.setString("MEGAKILL");
			this->map->sounds.push(SoundPlay{"megakill", 5, true, sf::Vector2i{0, 0}});
			break;
		case 5:
			this->scoreBonus = true;
			this->scoreBonusText.setString("BARBARIAN");
			this->map->sounds.push(SoundPlay{"barbarian", 5, true, sf::Vector2i{0, 0}});
			break;
		default: // >= 6
			this->scoreBonus = true;
			this->scoreBonusText.setString("BUTCHERY");
			this->map->sounds.push(SoundPlay{"butchery", 5, true, sf::Vector2i{0, 0}});
			break;
		}
	}

	void draw(sf::RenderWindow &window, float dt) {
		if (this->scoreBonus) {
			sf::FloatRect textRect = this->scoreBonusText.getLocalBounds();
			scoreBonusText.setOrigin(textRect.left + textRect.width / 2.0f,
			                         textRect.top  + textRect.height / 2.0f);
			scoreBonusText.setPosition(sf::Vector2f(this->screenWidth / 2, this->screenHeight / 2));
			window.draw(scoreBonusText);
		}
	}

	void clearStats() {
		auto playerView = this->vault->registry.view<Player>();

		for (EntityID entity : playerView) {
			Player &player = playerView.get(entity);
			// FIMXE: not sure we should clean that there
			player.kills.clear();
		}
	}

	void updateStats(float dt) {
		auto playerView = this->vault->registry.view<Player>();

		for (EntityID entity : playerView) {
			Player &player = playerView.get(entity);
#ifdef VICTORY_DEBUG
			std::cout << "Player " << entity << " " << player.team << " kills " << player.kills.size() << std::endl;
#endif
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
					for (auto pair : otherPlayer.objsByType) {
						ennemyObjs += pair.second.size();
					}
				}

			}
		}

		if (ennemyObjs == 0)
			return true;

		return false;
	}

};