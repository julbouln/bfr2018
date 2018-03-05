#pragma once

#include "Config.hpp"
#include "Events.hpp"

#include "GameVault.hpp"
#include "System.hpp"
#include "Map.hpp"

#include "third_party/entt/signal/dispatcher.hpp"

class GameSystem : public System {
public:
	Map *map;
	int screenWidth;
	int screenHeight;

	void setShared(GameVault *vault, Map *map, int screenWidth, int screenHeight) {
		this->vault = vault;
		this->map = map;
		this->screenWidth = screenWidth;
		this->screenHeight = screenHeight;

//		this->vault->dispatcher.connect<AnimationFrameChanged>(this);
	}

//	void receive(const AnimationFrameChanged &event) {
//		std::cout << "ANIMATION FRAME CHANGED RECEIVED "<< event.frame <<std::endl;
//	}

	sf::Vector2f tileDrawPosition(Tile &tile) const {
		return sf::Vector2f(tile.ppos.x - (tile.centerRect.left + tile.centerRect.width / 2) + tile.offset.x * 32,
		                    tile.ppos.y - (tile.centerRect.top + tile.centerRect.height / 2) + tile.offset.y * 32
		                   );
	}


	sf::Vector2i tilePosition(Tile &tile, sf::Vector2i p) const {
		return sf::Vector2i(tile.pos.x + (p.x - tile.size.x / 2) + tile.offset.x,
		                    tile.pos.y + (p.y - tile.size.y / 2) + tile.offset.y);
	}

	std::vector<sf::Vector2i> tileSurface(Tile &tile) const {
		std::vector<sf::Vector2i> surface;
		surface.reserve(tile.size.x * tile.size.y); // optimize by reserving expected vector size
		for (int w = 0; w < tile.size.x; ++w) {
			for (int h = 0; h < tile.size.y; ++h) {
				sf::Vector2i p = this->tilePosition(tile, sf::Vector2i(w, h));
				if (this->map->bound(p.x, p.y))
					surface.push_back(p);
			}
		}
		return surface;
	}

	std::vector<sf::Vector2i> vectorSurfaceExtended(sf::Vector2i pos, int dist) const {
		std::vector<sf::Vector2i> surface;
		surface.reserve((dist * 2) * (dist * 2)); // optimize by reserving expected vector size
		for (int w = -dist; w < dist + 1; ++w) {
			for (int h = -dist; h < dist + 1; ++h) {
				sf::Vector2i p(pos.x + w, pos.y + h);
				if (this->map->bound(p.x, p.y))
					surface.push_back(p);
			}
		}
		return surface;
	}

	std::vector<sf::Vector2i> tileSurfaceExtended(Tile &tile, int dist) const {
		std::vector<sf::Vector2i> surface;
		surface.reserve((tile.size.x + dist * 2) * (tile.size.y + dist * 2)); // optimize by reserving expected vector size
		for (int w = -dist; w < tile.size.x + dist; ++w) {
			for (int h = -dist; h < tile.size.y + dist; ++h) {
				sf::Vector2i p = this->tilePosition(tile, sf::Vector2i(w, h));
				if (this->map->bound(p.x, p.y) && distance(tile.pos, p) <= dist + length(sf::Vector2f(tile.size) / 2.0f) ) {
					surface.push_back(p);
				}
			}
		}
		return surface;
	}

	std::vector<sf::Vector2i> tileAround(Tile &tile, int minDist, int maxDist) const {
		// ellipse calculation
		std::vector<sf::Vector2i> surface;

		int minWidth = (tile.size.x) / 2 + minDist;
		int minHeight = (tile.size.y) / 2 + minDist;

		int maxWidth = tile.size.x / 2 + maxDist + 1;
		int maxHeight = tile.size.y / 2 + maxDist + 1;

		int min = minHeight * minHeight * minWidth * minWidth;
		int max = maxHeight * maxHeight * maxWidth * maxWidth;

		for (int y = -maxHeight + 1; y <= maxHeight - 1; y++) {
			for (int x = -maxWidth + 1; x <= maxWidth - 1; x++) {
				int rmin = x * x * minHeight * minHeight + y * y * minWidth * minWidth;
				int rmax = x * x * maxHeight * maxHeight + y * y * maxWidth * maxWidth;
				if (rmax <= max && rmin >= min)
					if (this->map->bound(tile.pos.x + x, tile.pos.y + y))
						surface.push_back(sf::Vector2i(tile.pos.x + x, tile.pos.y + y));
			}
		}

		return surface;
	}

	sf::Vector2i nearestTileAround(Tile &tile, Tile &destTile, int minDist, int maxDist) const {
		sf::Vector2i nearest(1024, 1024);
		for (sf::Vector2i const &p : this->tileAround(destTile, minDist, maxDist)) {
			if (this->map->pathAvailable(p.x, p.y)) {
				if (distance(tile.pos, p) < distance(tile.pos, nearest)) {
					nearest = p;
				}
			}
		}
		if (nearest.x == 1024 && nearest.y == 1024) {
			return tile.pos;
		}
		else
			return nearest;
	}

	sf::Vector2i firstAvailablePosition(sf::Vector2i src, int minDist, int maxDist) const {
		int dist = minDist;
		while (dist < maxDist) {
			for (int w = -dist; w < dist + 1; ++w) {
				for (int h = -dist; h < dist + 1; ++h) {
					if (w == -dist || h == -dist || w == dist || h == dist) {
						int x = w + src.x;
						int y = h + src.y;
						if (this->map->bound(x, y)) {
							if (this->map->positionAvailable(x, y))
								return sf::Vector2i(x, y);
						}
					}
				}
			}
			dist++;
		}
//		std::cout << "BUG: no available position for " << src.x << "x" << src.y << std::endl;
		return src;
	}

	EntityID ennemyAtPosition(EntityID playerEnt, int x, int y) {
		if (this->map->bound(x, y)) {
			Player &player = this->vault->registry.get<Player>(playerEnt);
			EntityID destEnt = this->map->objs.get(x, y);
			if (destEnt) {
				GameObject &obj = this->vault->registry.get<GameObject>(destEnt);
				if (obj.team != player.team)
					return destEnt;
			}
		}
		return 0;
	}

	bool ennemyInRange(Tile &tile, Tile &destTile, int range, int maxRange)
	{
		for (sf::Vector2i const &p : this->tileAround(tile, range, maxRange)) {
			for (sf::Vector2i const &dp : this->tileSurface(destTile)) {
				if (dp == p)
					return true;
			}
		}

		return false;
	}

	void addPlayerFrontPoint(EntityID playerEnt, EntityID ent, sf::Vector2i pos) {
		Player &player = this->vault->registry.get<Player>(playerEnt);
		if (this->vault->registry.has<Unit>(ent)) {
			if (distance(player.initialPos, pos) < distance(sf::Vector2i(0, 0), sf::Vector2i(this->map->width, this->map->height)) / 4) {
				player.allFrontPoints.push_back(pos);
			}
		} else {
			if (this->vault->registry.has<Building>(ent)) {
				player.allFrontPoints.push_back(pos);
			}
		}
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

	bool canSpendResources(EntityID playerEnt, std::string type, int val) {
		Player &player = this->vault->registry.get<Player>(playerEnt);
		if (player.resources > val)
			return true;
		else
			return false;
	}

	EntityID emitEffect(std::string name, sf::Vector2f ppos, ParticleEffectOptions options = ParticleEffectOptions()) {
		EntityID entity = this->vault->factory.createParticleEffect(this->vault->registry, name, options);
		ParticleEffect &effect = this->vault->registry.get<ParticleEffect>(entity);
		effect.spawner->center = ppos;
		if (!effect.continuous)
			effect.particleSystem->emitParticles(effect.particles);

		return entity;
	}

	EntityID emitEffect(std::string name, EntityID emitter, sf::Vector2f ppos, ParticleEffectOptions options = ParticleEffectOptions()) {
		if (this->vault->registry.has<Effects>(emitter)) {
			Effects &effects = this->vault->registry.get<Effects>(emitter);
			if (effects.effects.count(name) > 0) {
				EntityID entity = this->vault->factory.createParticleEffect(this->vault->registry, effects.effects[name], options);
				ParticleEffect &effect = this->vault->registry.get<ParticleEffect>(entity);
				effect.spawner->center = ppos;
				if (!effect.continuous)
					effect.particleSystem->emitParticles(effect.particles);
#ifdef GAME_SYSTEM_DEBUG
				std::cout << "GameSystem: emit effect " << name << " at " << ppos.x << "x" << ppos.y << std::endl;
#endif
				return entity;
			}
		}
		return 0;
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
				this->emitEffect("spend", entity, fxPos);

				this->vault->factory.destroyEntity(this->vault->registry, entity);

				if (spended <= 0)
					return;
			}
		}
	}

	float buildTime(std::string type) {
		Building building;
		this->vault->factory.parseBuildingFromXml(type, building);
		return building.maxBuildTime;
	}

	float trainCost(std::string type) {
		Unit unit;
		this->vault->factory.parseUnitFromXml(type, unit);
		return unit.cost;
	}

	// get object initial life from XML
	float objTypeLife(std::string type) {
		GameObject obj;
		this->vault->factory.parseGameObjectFromXml(type, obj);
		return obj.maxLife;
	}

	void resetAnim(EntityID entity, std::string state, int view) {
		if (this->vault->registry.has<AnimatedSpritesheet>(entity)) {
			AnimatedSpritesheet &anim = this->vault->registry.get<AnimatedSpritesheet>(entity);
			AnimatedSpriteView &animSprite = anim.states[state][view];
			animSprite.l = 0;
			animSprite.t = 0.0;
			animSprite.currentFrame = 0;
			animSprite.frameChangeCallback = [](int frame) {};
		}
	}

	void resetAnim(AnimatedSpritesheet &spritesheet, Tile &tile) {
		if (spritesheet.states.count(tile.state) > 0) {
			AnimatedSpriteView &view = spritesheet.states[tile.state][tile.view];
			view.l = 0;
			view.t = 0.0;
			view.currentFrame = 0;
			view.frameChangeCallback = [](int frame) {};
		}
	}

	void changeState(Tile & tile, std::string state) {
		if (tile.state != state) {
			tile.state = state;
		}
	}

	void changeState(EntityID entity, std::string state) {
		Tile &tile = this->vault->registry.get<Tile>(entity);
		if (tile.state != state) {
			if (this->vault->registry.has<AnimatedSpritesheet>(entity)) {
				AnimatedSpritesheet &spritesheet = this->vault->registry.get<AnimatedSpritesheet>(entity);
				this->resetAnim(spritesheet, tile);
			}
			tile.state = state;
		}
	}

	void playSound(sf::Sound & snd, std::string name) {
		snd.setBuffer(this->vault->factory.getSndBuf(name));
		snd.play();
	}

	void playRandomUnitSound(EntityID ent, std::string state) {
		Unit &unit = this->vault->registry.get<Unit>(ent);
		GameObject &obj = this->vault->registry.get<GameObject>(ent);

		this->playRandomUnitSound(obj, unit, state);
	}

	void playRandomUnitSound(GameObject & obj, Unit & unit, std::string state) {
		if (unit.soundActions[state] > 0) {
			int rnd = rand() % unit.soundActions[state];
			std::string sname = obj.name + "_" + state + "_" + std::to_string(rnd);
			this->map->sounds.push(SoundPlay{sname, 3, true, sf::Vector2i{0, 0}});
		}
	}

// action
	void seedResources(std::string type, EntityID entity) {
		if (this->vault->registry.valid(entity) && this->vault->registry.has<Tile>(entity)) { // FIXME: weird
			Tile &tile = this->vault->registry.get<Tile>(entity);
			for (sf::Vector2i const &p : this->tileAround(tile, 1, 2)) {
				float rnd = ((float) rand()) / (float) RAND_MAX;
				if (rnd > 0.85) {
					if (!this->map->resources.get(p.x, p.y) &&
					        !this->map->objs.get(p.x, p.y) && this->map->staticBuildable.get(p.x, p.y) == 0) {
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
			float cost = this->trainCost(type);

			if (this->canSpendResources(playerEnt, player.resourceType, cost)) {
				for (sf::Vector2i const &p : this->tileAround(tile, 1, 2)) {
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

	void sendGroup(std::vector<EntityID> group, sf::Vector2i destpos, GroupFormation formation, int direction, bool playSound) {
		if (group.size() > 0) {
			switch (formation) {
			case GroupFormation::Square:
			{
				double squareD = sqrt((double)group.size());
				int square = ceil(squareD);
				int cur = 0;
				for (int x = 0; x < square; x++) {
					for (int y = 0; y < square; y++) {
						if (cur < group.size()) {
							EntityID entity = group[cur];
							if (this->vault->registry.has<Unit>(entity)) {
								this->goTo(entity, sf::Vector2i(destpos.x + x, destpos.y + y));
								this->clearTarget(entity);
								if (playSound)
									this->playRandomUnitSound(entity, "move");
							}
							cur++;
						}
					}
				}
			}
			break;
			case GroupFormation::TwoLine:
			{
				int cur = 0;
				for (int x = 0; x < group.size() / 2; x++) {
					for (int y = 0; y < 2; y++) {
						if (cur < group.size()) {
							EntityID entity = group[cur];
							if (this->vault->registry.has<Unit>(entity)) {
								this->goTo(entity, sf::Vector2i(destpos.x + x, destpos.y + y));
								this->clearTarget(entity);
								if (playSound)
									this->playRandomUnitSound(entity, "move");
							}
							cur++;
						}
					}
				}
			}
			default:
				for (EntityID entity : group) {
					this->goTo(entity, destpos);
					this->clearTarget(entity);

				}
				break;
			}
		}
	}

	void clearTarget(Unit & unit) {
		unit.targetEnt = 0;
	}

	void clearTarget(EntityID entity) {
		Unit &unit = this->vault->registry.get<Unit>(entity);
		this->clearTarget(unit);
	}

	void goTo(Unit & unit, sf::Vector2i destpos) {
		if (this->map->positionAvailable(destpos.x, destpos.y) )
			unit.destpos = destpos;
		else
			unit.destpos = this->firstAvailablePosition(destpos, 1, 16);

		unit.pathUpdate = true;
		unit.commanded = true;
		unit.nopath = 0;
	}

	void goTo(EntityID entity, sf::Vector2i destpos) {
		Unit &unit = this->vault->registry.get<Unit>(entity);
		this->goTo(unit, destpos);
	}

	void stop(Unit &unit) {
	}

	void attack(Unit & unit, EntityID destEnt) {
		unit.targetEnt = destEnt;
	}

	void attack(EntityID entity, EntityID destEnt) {
		Unit &unit = this->vault->registry.get<Unit>(entity);
		this->attack(unit, destEnt);
	}


};