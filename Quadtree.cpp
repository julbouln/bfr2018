#include "Quadtree.hpp"

Quadtree::Quadtree(float _x, float _y, float _width, float _height, int _level, int _maxLevel) :
    x       (_x),
    y       (_y),
    width   (_width),
    height  (_height),
    level   (_level),
    maxLevel(_maxLevel)
{
    if (level == maxLevel)
        return;

    const float halfWidth  = width * 0.5f;
    const float halfHeight = height * 0.5f;

    NW = new Quadtree(x, y, halfWidth, halfHeight, level+1, maxLevel);
    NE = new Quadtree(x + halfWidth, y, halfWidth, halfHeight, level+1, maxLevel);
    SW = new Quadtree(x, y + halfHeight, halfWidth, halfHeight, level+1, maxLevel);
    SE = new Quadtree(x + halfWidth, y + halfHeight, halfWidth, halfHeight, level+1, maxLevel);
}

Quadtree::~Quadtree() {
    if (level == maxLevel)
        return;

    delete NW;
    delete NE;
    delete SW;
    delete SE;
}

void Quadtree::add(QuadtreeObject object) {
    if (level == maxLevel)
        objects.push_back(object);
    else if (contains(NW, &object))
        NW->add(object);
    else if (contains(NE, &object))
        NE->add(object);
    else if (contains(SW, &object))
        SW->add(object);
    else if (contains(SE, &object))
        SE->add(object);
    else if (contains(this, &object))
        objects.push_back(object);
}

std::vector<QuadtreeObject> Quadtree::getAt(float _x, float _y) {
   if (level == maxLevel) {
        return objects;
    }
    
    std::vector<QuadtreeObject> returnObjects, childReturnObjects;
    if (!objects.empty()) {
        returnObjects = objects;
    }
    if (_x > x + width / 2.0f && _x < x + width) {
        if (_y > y + height / 2.0f && _y < y + height) {
            childReturnObjects = SE->getAt(_x, _y);
            returnObjects.insert(returnObjects.end(), childReturnObjects.begin(), childReturnObjects.end());
            return returnObjects;
        } else if (_y > y && _y <= y + height / 2.0f) {
            childReturnObjects = NE->getAt(_x, _y);
            returnObjects.insert(returnObjects.end(), childReturnObjects.begin(), childReturnObjects.end());
            return returnObjects;
        }
    } else if (_x > x && _x <= x + width / 2.0f) {
        if (_y > y + height / 2.0f && _y < y + height) {
            childReturnObjects = SW->getAt(_x, _y);
            returnObjects.insert(returnObjects.end(), childReturnObjects.begin(), childReturnObjects.end());
            return returnObjects;
        } else if (_y > y && _y <= y + height / 2.0f) {
            childReturnObjects = NW->getAt(_x, _y);
            returnObjects.insert(returnObjects.end(), childReturnObjects.begin(), childReturnObjects.end());
            return returnObjects;
        }
    }
    return returnObjects;
}

std::vector<QuadtreeObject> Quadtree::getAt(float _x, float _y, float _width, float _height) {
    std::vector<QuadtreeObject> returnObjects, childReturnObjects;

    if (level == maxLevel)
        return objects;

    if (!objects.empty())
        returnObjects = objects;

    const float halfWidth  = width * 0.5f;
    const float halfHeight = height * 0.5f;

    if (_x > x + halfWidth && _x + _width < x + width)
        if (_y > y + halfHeight && _y + _height < y + height)
            childReturnObjects = SE->getAt(_x, _y);
        else if (_y > y && _y + _height <= y + halfHeight)
            childReturnObjects = NE->getAt(_x, _y);
    else if (_x > x && _x + _width <= x + halfWidth)
        if (_y > y + halfHeight && _y + _height < y + height)
            childReturnObjects = SW->getAt(_x, _y);
        else if (_y > y && _y + _height <= y + halfHeight)
            childReturnObjects = NW->getAt(_x, _y);

    if (childReturnObjects.size() > 0)
        returnObjects.insert(returnObjects.end(), childReturnObjects.begin(), childReturnObjects.end());

    return returnObjects;
}

void Quadtree::clear() {
    if (level == maxLevel) {
        objects.clear();
        return;
    } else {
        NW->clear();
        NE->clear();
        SW->clear();
        SE->clear();
    }
    if (!objects.empty()) {
        objects.clear();
    }
}

bool Quadtree::contains(Quadtree *child, QuadtreeObject *object) {
    return !(object->x < child->x ||
             object->y < child->y ||
             object->x > child->x + child->width  ||
             object->y > child->y + child->height ||
             object->x + object->width  < child->x ||
             object->y + object->height < child->y ||
             object->x + object->width  > child->x + child->width ||
             object->y + object->height > child->y + child->height);
}
