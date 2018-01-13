#pragma once

#include "Node.h"

namespace BrainTree
{

class Decorator : public Node
{
public:
    virtual ~Decorator() {}

    void setChild(std::shared_ptr<Node> node) { child = node; }
    bool hasChild() const { return child != nullptr; }
    
protected:
    std::shared_ptr<Node> child = nullptr;
};

}