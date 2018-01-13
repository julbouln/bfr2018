#pragma once

#include "Node.h"
#include "Blackboard.h"

namespace BrainTree
{

class BehaviorTree : public Node
{
public:
    BehaviorTree() : blackboard(std::make_shared<Blackboard>()) {}
    BehaviorTree(const Node::Ptr &rootNode) : BehaviorTree() { root = rootNode; }
    BehaviorTree(const Blackboard::Ptr &shared) : BehaviorTree() { sharedBlackboard = shared; }
    
    Status update() { 
        if(root)
            return root->tick(); 
        else
            return Status::Failure;
    }
    
    void setRoot(const Node::Ptr &node) { root = node; }
    bool hasRoot(const Node::Ptr &node) { if(root) return true; else return false; }
    Blackboard::Ptr getBlackboard() const { return blackboard; }
    Blackboard::Ptr getSharedBlackboard() const { return sharedBlackboard; }
    void setSharedBlackboard(const Blackboard::Ptr &shared) { sharedBlackboard = shared; }
    
private:
    Node::Ptr root = nullptr;
    Blackboard::Ptr blackboard = nullptr;
    Blackboard::Ptr sharedBlackboard = nullptr;
};

}
