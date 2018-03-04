#pragma once

#include <vector>
#include "Entity.hpp"
#include "SFML/Graphics/Rect.hpp"

template <typename T>
class Quadtree;

/*
class QuadtreeObject : public sf::FloatRect {
public:
    EntityID entity;

    QuadtreeObject(EntityID ent, float x, float y, float w, float h) : sf::FloatRect(x, y, w, h), entity(ent) {}
};
*/

template <typename T>
class Quadtree : public sf::FloatRect {
public:

    Quadtree(float _x, float _y, float _width, float _height, int _level, int _maxLevel) :
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


    Quadtree(float _x, float _y, float _width, float _height, int minRectWidth) :
        sf::FloatRect(_x, _y, _width, _height)
    {
        this->level = 0;
        this->maxLevel = 0;
        int calcWidth = _width;
        while (calcWidth > minRectWidth) {
            calcWidth = calcWidth * 0.5f;
            this->maxLevel++;
        }

        if (level == maxLevel)
            return;

        float halfWidth  = this->halfWidth();
        float halfHeight = this->halfHeight();

//    std::cout << "Quadtree: create " << level << "/" << maxLevel << " " << halfWidth << "x" << halfHeight << std::endl;

        NW = new Quadtree<T>(this->left, this->top, halfWidth, halfHeight, level + 1, maxLevel);
        NE = new Quadtree<T>(this->centerX(), this->top, halfWidth, halfHeight, level + 1, maxLevel);
        SW = new Quadtree<T>(this->left, this->centerY(), halfWidth, halfHeight, level + 1, maxLevel);
        SE = new Quadtree<T>(this->centerX(), this->centerY(), halfWidth, halfHeight, level + 1, maxLevel);
    }

    ~Quadtree() {
        if (level == maxLevel)
            return;

        delete NW;
        delete NE;
        delete SW;
        delete SE;
    }

    void add(T object) {
#ifdef QUADTREE_DEBUG
        std::cout << "Quadtree: add " << object.entity << " " << object.left << "x" << object.top << " -> " << this->level << " " << this->left << "x" << this->top << ":" << this->width << "x" << this->height << std::endl;
#endif
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

    std::vector<T> getAt(float _x, float _y) {
        if (level == maxLevel)
            return objects;

        std::vector<T> returnObjects, childReturnObjects;
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

    std::vector<T> getAt(float _x, float _y, float _width, float _height) {
        if (level == maxLevel)
            return objects;

        std::vector<T> returnObjects, childReturnObjects;
        if (!objects.empty()) {
            returnObjects = objects;
        }

        if (SE->contains(_x, _y, _width, _height) || SE->intersects(_x, _y, _width, _height)) {
            childReturnObjects = SE->getAt(_x, _y, _width, _height);
            returnObjects.insert(returnObjects.end(), childReturnObjects.begin(), childReturnObjects.end());
        }
        if (NE->contains(_x, _y, _width, _height) || NE->intersects(_x, _y, _width, _height)) {
            childReturnObjects = NE->getAt(_x, _y, _width, _height);
            returnObjects.insert(returnObjects.end(), childReturnObjects.begin(), childReturnObjects.end());
        }
        if (SW->contains(_x, _y, _width, _height) || SW->intersects(_x, _y, _width, _height)) {
            childReturnObjects = SW->getAt(_x, _y, _width, _height);
            returnObjects.insert(returnObjects.end(), childReturnObjects.begin(), childReturnObjects.end());
        }
        if (NW->contains(_x, _y, _width, _height) || NW->intersects(_x, _y, _width, _height)) {
            childReturnObjects = NW->getAt(_x, _y, _width, _height);
            returnObjects.insert(returnObjects.end(), childReturnObjects.begin(), childReturnObjects.end());
        }

#ifdef QUADTREE_DEBUG
        std::cout << "Quadtree: getAt " << _x << "x" << _y << ":" << _width << "x" << _height << " : " << returnObjects.size() << " " << this->level << " " << this->left << "x" << this->top << ":" << this->width << "x" << this->height << std::endl;
#endif

        return returnObjects;
    }

    void clear() {
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

    inline bool     contains(float x, float y) const {return sf::FloatRect::contains(x, y);};
    inline bool     contains(float x, float y, float w, float h) const {return this->contains(x, y) && this->contains(x + w, y + h);};
    inline bool     contains(T &object) const {return this->contains(object.left, object.top, object.width, object.height);};

    inline bool     intersects(float x, float y, float w, float h) const {return sf::FloatRect::intersects(sf::FloatRect(x, y, w, h));};

private:
//    sf::FloatRect   rect;

    int             level;
    int             maxLevel;
    std::vector<T> objects;

    Quadtree<T> *      NW;
    Quadtree<T> *      NE;
    Quadtree<T> *      SW;
    Quadtree<T> *      SE;

    inline float halfWidth() const { return this->width * 0.5f; };
    inline float halfHeight() const { return this->height * 0.5f; };
    inline float centerX() const { return this->left + this->width * 0.5f;};
    inline float centerY() const { return this->top + this->height * 0.5f;};
    inline float right() const { return this->left + this->width;};
    inline float bottom() const { return this->top + this->height;};

};
