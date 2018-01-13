#pragma once

#include "../Composite.h"

namespace BrainTree
{

// The Selector composite ticks each child node in order.
// If a child succeeds or runs, the sequence returns the same status.
// In the next tick, it will try to run each child in order again.
// If all children fails, only then does the selector fail.
class RandomSelector : public Selector
{
public:
    std::vector<int> shuffled;

    void initialize() override
    {
        index = 0;
        while (this->shuffled.size() < children.size())
            this->shuffled.push_back(shuffled.size());

        std::random_shuffle ( this->shuffled.begin(), this->shuffled.end() );
    }

    Status update() override
    {
        if (!hasChildren()) {
            return Status::Success;
        }

        // Keep going until a child behavior says it's running.
        while (1) {
            auto &child = children.at(shuffled.at(index));
            auto status = child->tick();

            // If the child succeeds, or keeps running, do the same.
            if (status != Status::Failure) {
                return status;
            }

            // Hit the end of the array, it didn't end well...
            if (++index == children.size()) {
                return Status::Failure;
            }
        }
    }
};

}