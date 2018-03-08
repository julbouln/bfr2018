#include <iostream>
#define QUADTREE_DEBUG
#include "Quadtree.hpp"

class QuadtreeObject : public sf::Vector2f {
public:
    EntityID entity;

    QuadtreeObject(EntityID ent, float x, float y) : sf::Vector2f(x, y), entity(ent) {}
};


int main() {
	Quadtree<QuadtreeObject> qt(0,0,96*32,96*32,16);

	sf::FloatRect rect1(0,0,64,64);
	sf::FloatRect rect2(16,16,32,32);

	std::cout << "rect1 <=> rect2 " << rect1.intersects(rect2) << "?" <<std::endl;
	std::cout << "rect2 <=> rect1 " << rect2.intersects(rect1) << "?" <<std::endl;

	qt.add(QuadtreeObject(1,4,8));
	qt.add(QuadtreeObject(2,4,30));
	qt.add(QuadtreeObject(3,31,31));
	qt.add(QuadtreeObject(4,33,33));
	qt.add(QuadtreeObject(5,64,64));
	qt.add(QuadtreeObject(6,128,64));
	qt.add(QuadtreeObject(7,256,64));
	qt.add(QuadtreeObject(8,512,64));
	qt.add(QuadtreeObject(9,-16,-16));

	std::vector<QuadtreeObject> rects=qt.get(0,0,32,32);
	for(auto &rect : rects) {
		std::cout << "FOUND "<< rect.entity << ": "<<rect.x<<"x"<<rect.y<< std::endl;
	}

	std::vector<QuadtreeObject> rects2;
	qt.retrieve(rects2, 0,0,32,32);
	for(auto &rect : rects2) {
		std::cout << "FOUND "<< rect.entity << ": "<<rect.x<<"x"<<rect.y<< std::endl;
	}

}