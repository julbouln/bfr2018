#pragma once

#include "Node.h"
#include <vector>

namespace BrainTree
{

class Composite : public Node
{
public:
    virtual ~Composite() {}
    
    void addChild(std::shared_ptr<Node> child) { children.push_back(child); }
    bool hasChildren() const { return !children.empty(); }
    int getIndex() const { return index; }
    
protected:
    std::vector<std::shared_ptr<Node>> children;
    int index = 0;
};

}