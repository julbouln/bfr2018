#pragma once

#include "GameSystem.hpp"
#include "TileMap.hpp"

class DrawMapSystem : public GameSystem {
public:
	std::vector<EntityID> entitiesDrawList;
	TileMap terrainsTileMap;
	TileMap fogTileMap;

	DrawMapSystem();

	void draw(sf::RenderWindow &window, sf::IntRect clip, float dt);
	void update(float dt);

	void initTileMaps();
	void updateAllTileMaps();

	void drawFogTileMap(sf::RenderWindow &window, float dt);

private:
	inline bool clipped(sf::IntRect & clip, sf::Vector2i const & p) const {
		return (p.x >= clip.left && p.x <= clip.left + clip.width &&
		        p.y >= clip.top && p.y <= clip.top + clip.height);
	}

	// reduce object list to visible entities
	void updateObjsDrawList(sf::RenderWindow & window, sf::IntRect clip, float dt);

	void updateAllTerrainTileMap(float dt);
	void updateAllFogTileMap(float dt);

	void drawSpriteWithShader(sf::RenderTarget & target, sf::Sprite & sprite, std::string shaderName, ShaderOptions & options);
	void drawEntityLayer(sf::RenderTarget & target, Layer<EntityID> & layer, sf::IntRect clip, float dt, sf::Color colorVariant = sf::Color(0xff, 0xff, 0xff));
	void drawTileLayers(sf::RenderTarget & target, sf::IntRect clip, float dt);
	void drawObjLayer(sf::RenderWindow & window, sf::IntRect clip, float dt);
	// draw debug grid
	void drawDebug(sf::RenderWindow & window, sf::IntRect clip, float dt);

	void drawTerrainTileMap(sf::RenderWindow &window, float dt);

};
