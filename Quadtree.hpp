#pragma once

#include <vector>
#include "Entity.hpp"

class Quadtree;
class QuadtreeObject {
public:
    EntityID entity;
    float                   x;
    float                   y;
    float                   width;
    float                   height;
};

class Quadtree {
public:
    Quadtree(float x, float y, float width, float height, int level, int maxLevel);
    ~Quadtree();

    void            add(QuadtreeObject object);
    std::vector<QuadtreeObject> getAt(float x, float y);
    std::vector<QuadtreeObject> getAt(float x, float y, float width, float height);
    void            clear();

private:
    float           x;
    float           y;
    float           width;
    float           height;
    int             level;
    int             maxLevel;
    std::vector<QuadtreeObject> objects;

    Quadtree *      NW;
    Quadtree *      NE;
    Quadtree *      SW;
    Quadtree *      SE;

    bool            contains(Quadtree *child, QuadtreeObject *object);
};
