#pragma once

#include <vector>
#include "Entity.hpp"
#include "SFML/Graphics/Rect.hpp"

class Quadtree;
class QuadtreeObject : public sf::FloatRect {
public:
    EntityID entity;

    QuadtreeObject(EntityID ent, float x, float y, float w, float h) : sf::FloatRect(x,y,w,h), entity(ent) {}
};

class Quadtree : public sf::FloatRect {
public:
    Quadtree(float x, float y, float width, float height, int level, int maxLevel);
    Quadtree(float x, float y, float width, float height, int minRectWidth);
    ~Quadtree();

    void            add(QuadtreeObject object);
    std::vector<QuadtreeObject> getAt(float x, float y);
    std::vector<QuadtreeObject> getAt(float x, float y, float width, float height);
    void            clear();

    inline bool     contains(float x, float y) const {return sf::FloatRect::contains(x,y);};
    inline bool     contains(float x, float y, float w, float h) const {return this->contains(x,y) && this->contains(x+w,y+h);};
    inline bool     contains(QuadtreeObject &object) const {return this->contains(object.left,object.top,object.width,object.height);};

private:
//    sf::FloatRect   rect;

    int             level;
    int             maxLevel;
    std::vector<QuadtreeObject> objects;

    Quadtree *      NW;
    Quadtree *      NE;
    Quadtree *      SW;
    Quadtree *      SE;

    inline float halfWidth() const { return this->width * 0.5f; };
    inline float halfHeight() const { return this->height * 0.5f; };
    inline float centerX() const { return this->left + this->width * 0.5f;};
    inline float centerY() const { return this->top + this->height * 0.5f;};
    inline float right() const { return this->left + this->width;};
    inline float bottom() const { return this->top + this->height;};

};
