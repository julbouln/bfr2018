#pragma once

#include "Config.hpp"

#include "GameVault.hpp"
#include "System.hpp"
#include "Map.hpp"

//#define GAME_SYSTEM_DEBUG

class GameSystem : public System {
public:
	Map *map;

	sf::Vector2f tileDrawPosition(Tile &tile) {
		return sf::Vector2f(tile.ppos.x - (tile.centerRect.left + tile.centerRect.width / 2) + 16 + tile.offset.x * 32,
		                    tile.ppos.y - (tile.centerRect.top + tile.centerRect.height / 2) + 16 + tile.offset.y * 32
		                   );
	}

	sf::Vector2i tilePosition(Tile &tile, sf::Vector2i p) {
		return sf::Vector2i(tile.pos.x + (p.x - tile.size.x / 2) + tile.offset.x,
		                    tile.pos.y + (p.y - tile.size.y / 2) + tile.offset.y);
	}

	std::vector<sf::Vector2i> tileSurface(Tile &tile) {
		std::vector<sf::Vector2i> surface;
		for (int w = 0; w < tile.size.x; ++w) {
			for (int h = 0; h < tile.size.y; ++h) {
				sf::Vector2i p = this->tilePosition(tile, sf::Vector2i(w, h));
				if (this->map->bound(p.x, p.y))
					surface.push_back(p);
			}
		}
		return surface;
	}

	std::vector<sf::Vector2i> vectorSurfaceExtended(sf::Vector2i pos, int dist) {
		std::vector<sf::Vector2i> surface;
		for (int w = -dist; w < dist + 1; ++w) {
			for (int h = -dist; h < dist + 1; ++h) {
				sf::Vector2i p(pos.x + w, pos.y + h);
				if (this->map->bound(p.x, p.y))
					surface.push_back(p);
			}
		}
		return surface;
	}

	std::vector<sf::Vector2i> tileSurfaceExtended(Tile &tile, int dist) {
		std::vector<sf::Vector2i> surface;
		for (int w = -dist; w < tile.size.x + dist; ++w) {
			for (int h = -dist; h < tile.size.y + dist; ++h) {
				sf::Vector2i p = this->tilePosition(tile, sf::Vector2i(w, h));
				if (this->map->bound(p.x, p.y))
					surface.push_back(p);
			}
		}
		return surface;
	}

	std::vector<sf::Vector2i> tileAround(Tile &tile, int dist) {
		std::vector<sf::Vector2i> surface;
		for (int w = -dist; w < tile.size.x + dist; ++w) {
			for (int h = -dist; h < tile.size.y + dist; ++h) {
				if (w <= -dist || h <= -dist || w >= tile.size.x || h >= tile.size.y) {
					sf::Vector2i p = this->tilePosition(tile, sf::Vector2i(w, h));
					if (this->map->bound(p.x, p.y))
						surface.push_back(p);
				}
			}
		}
		return surface;
	}

	sf::Vector2i nearestTileAround(sf::Vector2i src, Tile &tile, int dist) {
		sf::Vector2i nearest(1024, 1024);
		for (sf::Vector2i const &p : this->tileAround(tile, dist)) {
			if (!this->map->objs.get(p.x, p.y)) {
				if (this->approxDistance(src, p) < this->approxDistance(src, nearest)) {
					nearest = p;
				}
			}
		}
#ifdef BUG_DEBUG
		if (nearest.x == 1024 && nearest.y == 1024) {
			std::cout << "BUG: no nearest pos around " << tile.pos.x << "x" << tile.pos.y << std::endl;
		}
#endif
		return nearest;
	}

	sf::Vector2i firstFreePosition(sf::Vector2i src) {
		sf::Vector2i fp;
		int dist = 1;
		while (dist < 16) {
			for (int w = -dist; w < dist + 1; ++w) {
				for (int h = -dist; h < dist + 1; ++h) {
					if (w == -dist || h == -dist || w == dist || h == dist) {
						int x = w + src.x;
						int y = h + src.y;
						if (this->map->bound(x, y)) {
							if (!this->map->objs.get(x, y))
								return sf::Vector2i(x, y);
						}
					}
				}
			}
			dist++;
		}
	}

	EntityID ennemyAtPosition(EntityID playerEnt, int x, int y) {
		Player &player = this->vault->registry.get<Player>(playerEnt);
		EntityID destEnt = this->map->objs.get(x, y);
		if (destEnt) {
			GameObject &obj = this->vault->registry.get<GameObject>(destEnt);
			if (obj.team != player.team)
				return destEnt;
		}
		return 0;
	}

	std::vector<sf::Vector2i> canBuild(EntityID playerEnt, EntityID entity) {
		Tile &tile = this->vault->registry.get<Tile>(entity);
		Player &player = this->vault->registry.get<Player>(playerEnt);
		std::vector<sf::Vector2i> restrictedPos;

		for (sf::Vector2i p : this->tileSurface(tile)) {
			EntityID pEnt = this->map->objs.get(p.x, p.y);
			if ((pEnt && pEnt != entity) || player.fog.get(p.x, p.y) == FogState::Unvisited || this->map->staticBuildable.get(p.x, p.y) != 0)
			{
				restrictedPos.push_back(p);
			}
		}

		return restrictedPos;
	}

	sf::Vector2f dirMovement(int direction, float speed) {
		sf::Vector2f mov(0.0, 0.0);
		switch (direction) {
		case North:
			mov.y -= speed;
			break;
		case NorthEast:
			mov.x += speed;
			mov.y -= speed;
			break;
		case East:
			mov.x += speed;
			break;
		case SouthEast:
			mov.x += speed;
			mov.y += speed;
			break;
		case South:
			mov.y += speed;
			break;
		case NorthWest:
			mov.x -= speed;
			mov.y -= speed;
			break;
		case West:
			mov.x -= speed;
			break;
		case SouthWest:
			mov.x -= speed;
			mov.y += speed;
			break;
		default:
			break;
		}
		return mov;
	}


	std::vector<sf::Vector2f> lineTrajectory( int x0, int y0, int x1, int y1)
	{
		std::vector<sf::Vector2f> points;
		float dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
		float dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
		float err = (dx > dy ? dx : -dy) / 2, e2;

		for (;;) {
			points.push_back(sf::Vector2f(x0, y0));
			if (x0 == x1 && y0 == y1) break;
			e2 = err;
			if (e2 > -dx) { err -= dy; x0 += sx; }
			if (e2 < dy) { err += dx; y0 += sy; }
		}
		return points;
	}

	bool canSpendResources(EntityID playerEnt, std::string type, int val) {
		Player &player = this->vault->registry.get<Player>(playerEnt);
		if (player.resources > val)
			return true;
		else
			return false;
	}

	void emitEffect(std::string name, EntityID emitter, sf::Vector2f ppos, float lifetime, ParticleEffectOptions options=ParticleEffectOptions{}) {
		if (this->vault->registry.has<Effects>(emitter)) {
			Effects &effects = this->vault->registry.get<Effects>(emitter);
			if (effects.effects.count(name) > 0) {
				EntityID entity = this->vault->factory.createParticleEffect(this->vault->registry, effects.effects[name], lifetime, options);
				ParticleEffect &effect = this->vault->registry.get<ParticleEffect>(entity);
				effect.spawner->center = ppos;
				effect.particleSystem->emitParticles(effect.particles);
#ifdef GAME_SYSTEM_DEBUG
				std::cout << "GameSystem: emit effect " << name << " at " << ppos.x << "x" << ppos.y << std::endl;
#endif
			}
		}

	}

	void spendResources(EntityID playerEnt, std::string type, int val) {
		Player &player = this->vault->registry.get<Player>(playerEnt);
		int spended = val;
		auto view = this->vault->registry.view<Resource>();
#ifdef GAME_SYSTEM_DEBUG
		std::cout << "GameSystem: spend " << val << " " << (int)type << std::endl;
#endif
		for (EntityID entity : view) {
			Resource &resource = view.get(entity);
			if (resource.type == type && resource.level > 0) {
				spended -= resource.level;
				Tile &tile = this->vault->registry.get<Tile>(entity);
				sf::Vector2f fxPos = tile.ppos;
				fxPos.x += 16.0;
				fxPos.y += 16.0;
				this->emitEffect("spend", entity, fxPos, 5.0);

				this->vault->factory.destroyEntity(this->vault->registry, entity);

				if (spended <= 0)
					return;
			}
		}
	}

	// get object initial life from XML
	float objTypeLife(std::string type) {
		GameObject obj;
		this->vault->factory.parseGameObjectFromXml(type, obj);
		return obj.life;
	}

	void changeState(Tile &tile, std::string state) {
		if (tile.state != state) {
			tile.state = state;
			this->vault->factory.resetTileAnim(tile, state);
		}
	}

	void playSound(sf::Sound &snd, std::string name) {
		snd.setBuffer(this->vault->factory.getSndBuf(name));
		snd.play();
	}

	void playRandomUnitSound(EntityID ent, std::string state) {
		Unit &unit = this->vault->registry.get<Unit>(ent);
		GameObject &obj = this->vault->registry.get<GameObject>(ent);

		this->playRandomUnitSound(obj, unit, state);
	}

	void playRandomUnitSound(GameObject &obj, Unit &unit, std::string state) {
		if (unit.soundActions[state] > 0) {
			int rnd = rand() % unit.soundActions[state];
			std::string sname = obj.name + "_" + state + "_" + std::to_string(rnd);
			if (this->map->sounds.size() < MAX_SOUNDS)
				this->map->sounds.push(SoundPlay{sname, 1, true, sf::Vector2i{0, 0}});
		}
	}

// action
	void seedResources(std::string type, EntityID entity) {
		if (this->vault->registry.valid(entity) && this->vault->registry.has<Tile>(entity)) { // FIXME: weird
			Tile &tile = this->vault->registry.get<Tile>(entity);
			for (sf::Vector2i const &p : this->tileAround(tile, 1)) {
				float rnd = ((float) rand()) / (float) RAND_MAX;
				if (rnd > 0.85) {
					if (!this->map->resources.get(p.x, p.y) && !this->map->objs.get(p.x, p.y) && this->map->staticBuildable.get(p.x, p.y) == 0) {
//					std::cout << " seed "<<(int)type<< " at "<<p.x<<"x"<<p.y<<std::endl;
						EntityID resEnt = this->vault->factory.plantResource(this->vault->registry, type, p.x, p.y);
						this->map->resources.set(p.x, p.y, resEnt);
					}
				}
			}
		} else {
#ifdef BUG_DEBUG
			std::cout << "BUG: seedResources entity " << entity << " invalid or does not has Tile" << std::endl;
#endif
		}
	}

	bool trainUnit(std::string type, EntityID playerEnt, EntityID entity ) {
		if (this->vault->registry.valid(entity) && this->vault->registry.has<Tile>(entity)) { // FIXME: weird
			Player &player = this->vault->registry.get<Player>(playerEnt);
			Tile &tile = this->vault->registry.get<Tile>(entity);
			float cost = 2 * this->objTypeLife(type);

			if (this->canSpendResources(playerEnt, player.resourceType, cost)) {
				for (sf::Vector2i const &p : this->tileAround(tile, 1)) {
					if (!this->map->objs.get(p.x, p.y)) {
						EntityID newEnt = this->vault->factory.createUnit(this->vault->registry, playerEnt, type, p.x, p.y);
						this->spendResources(playerEnt, player.resourceType, cost);
						player.resources -= cost;
#ifdef GAME_SYSTEM_DEBUG
						std::cout << "GameSystem: train " << type << std::endl;
#endif
						return true;
					}
				}
			}
		} else {
#ifdef BUG_DEBUG
			std::cout << "BUG: trainUnit entity " << entity << " invalid or does not has Tile" << std::endl;
#endif
		}
		return false;
	}

	void goTo(Unit &unit, sf::Vector2i destpos) {
		unit.destpos = destpos;
		unit.nopath = 0;
	}

	void goTo(EntityID entity, sf::Vector2i destpos) {
		Unit &unit = this->vault->registry.get<Unit>(entity);
		this->goTo(unit, destpos);
	}

	void attack(Unit &unit, EntityID destEnt) {
		unit.destAttack = destEnt;
	}

	void attack(EntityID entity, EntityID destEnt) {
		Unit &unit = this->vault->registry.get<Unit>(entity);
		this->attack(unit, destEnt);
	}

};