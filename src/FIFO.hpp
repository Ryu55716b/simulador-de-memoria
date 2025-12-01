#pragma once
#include "ReplacementAlgorithm.hpp"
#include <queue>

class FIFO : public ReplacementAlgorithm {
    std::queue<int> q;

public:
    void init(int) override { while(!q.empty()) q.pop(); }

    int selectVictim(const std::vector<int>& ram_owner) override {
        while (!q.empty() && (q.front() >= (int)ram_owner.size() || ram_owner[q.front()] == -1))
            q.pop();

        int victim = q.empty() ? 0 : q.front();
        if (!q.empty()) q.pop();
        q.push(victim);
        return victim;
    }

    void accessPage(int) override {}
    void pageRemoved(int) override {}
    std::string name() const override { return "FIFO"; }
};