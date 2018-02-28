#include <iostream>
#include "Quadtree.hpp"

Quadtree::Quadtree(float _x, float _y, float _width, float _height, int _level, int _maxLevel) :
    sf::FloatRect(_x, _y, _width, _height),
    level   (_level),
    maxLevel(_maxLevel)
{
    if (level == maxLevel)
        return;

    float halfWidth  = this->halfWidth();
    float halfHeight = this->halfHeight();

//    std::cout << "Quadtree: create " << level << "/" << maxLevel << " " << halfWidth << "x" << halfHeight << std::endl;

    NW = new Quadtree(this->left, this->top, halfWidth, halfHeight, level + 1, maxLevel);
    NE = new Quadtree(this->centerX(), this->top, halfWidth, halfHeight, level + 1, maxLevel);
    SW = new Quadtree(this->left, this->centerY(), halfWidth, halfHeight, level + 1, maxLevel);
    SE = new Quadtree(this->centerX(), this->centerY(), halfWidth, halfHeight, level + 1, maxLevel);
}


Quadtree::Quadtree(float _x, float _y, float _width, float _height, int minRectWidth) :
    sf::FloatRect(_x, _y, _width, _height)
{
    this->level = 0;
    this->maxLevel = 0;
    int calcWidth = _width;
    while(calcWidth > minRectWidth) {
        calcWidth = calcWidth * 0.5f;
        this->maxLevel++;
    }

    if (level == maxLevel)
        return;

    float halfWidth  = this->halfWidth();
    float halfHeight = this->halfHeight();

//    std::cout << "Quadtree: create " << level << "/" << maxLevel << " " << halfWidth << "x" << halfHeight << std::endl;

    NW = new Quadtree(this->left, this->top, halfWidth, halfHeight, level + 1, maxLevel);
    NE = new Quadtree(this->centerX(), this->top, halfWidth, halfHeight, level + 1, maxLevel);
    SW = new Quadtree(this->left, this->centerY(), halfWidth, halfHeight, level + 1, maxLevel);
    SE = new Quadtree(this->centerX(), this->centerY(), halfWidth, halfHeight, level + 1, maxLevel);
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
    else if (NW->contains(object))
        NW->add(object);
    else if (NE->contains(object))
        NE->add(object);
    else if (SW->contains(object))
        SW->add(object);
    else if (SE->contains(object))
        SE->add(object);
    else if (this->contains(object))
        objects.push_back(object);
}

std::vector<QuadtreeObject> Quadtree::getAt(float _x, float _y) {
    if (level == maxLevel)
        return objects;

    std::vector<QuadtreeObject> returnObjects, childReturnObjects;
    if (!objects.empty()) {
        returnObjects = objects;
    }

    if (SE->contains(_x, _y)) {
        childReturnObjects = SE->getAt(_x, _y);
        returnObjects.insert(returnObjects.end(), childReturnObjects.begin(), childReturnObjects.end());
    }
    if (NE->contains(_x, _y)) {
        childReturnObjects = NE->getAt(_x, _y);
        returnObjects.insert(returnObjects.end(), childReturnObjects.begin(), childReturnObjects.end());
    }
    if (SW->contains(_x, _y)) {
        childReturnObjects = SW->getAt(_x, _y);
        returnObjects.insert(returnObjects.end(), childReturnObjects.begin(), childReturnObjects.end());
    }
    if (NW->contains(_x, _y)) {
        childReturnObjects = NW->getAt(_x, _y);
        returnObjects.insert(returnObjects.end(), childReturnObjects.begin(), childReturnObjects.end());
    }
    
//    std::cout << "Quadtree: get "<<returnObjects.size()<<std::endl;
    return returnObjects;
}

std::vector<QuadtreeObject> Quadtree::getAt(float _x, float _y, float _width, float _height) {
if (level == maxLevel)
        return objects;

    std::vector<QuadtreeObject> returnObjects, childReturnObjects;
    if (!objects.empty()) {
        returnObjects = objects;
    }

    if (SE->contains(_x, _y, _width, _height)) {
        childReturnObjects = SE->getAt(_x, _y);
        returnObjects.insert(returnObjects.end(), childReturnObjects.begin(), childReturnObjects.end());
    }
    if (NE->contains(_x, _y, _width, _height)) {
        childReturnObjects = NE->getAt(_x, _y);
        returnObjects.insert(returnObjects.end(), childReturnObjects.begin(), childReturnObjects.end());
    }
    if (SW->contains(_x, _y, _width, _height)) {
        childReturnObjects = SW->getAt(_x, _y);
        returnObjects.insert(returnObjects.end(), childReturnObjects.begin(), childReturnObjects.end());
    }
    if (NW->contains(_x, _y, _width, _height)) {
        childReturnObjects = NW->getAt(_x, _y);
        returnObjects.insert(returnObjects.end(), childReturnObjects.begin(), childReturnObjects.end());
    }
    
//    std::cout << "Quadtree: get "<<returnObjects.size()<<std::endl;
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
