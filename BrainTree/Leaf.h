#pragma once

#include "Node.h"
#include "Blackboard.h"

namespace BrainTree
{

class Leaf : public Node
{
public:
    Leaf() {}
    virtual ~Leaf() {}
    Leaf(std::shared_ptr<Blackboard> blackboard) : blackboard(blackboard) {}
    
    virtual Status update() = 0;

protected:
    std::shared_ptr<Blackboard> blackboard;
};

}
