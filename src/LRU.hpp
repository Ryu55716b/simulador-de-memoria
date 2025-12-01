#pragma once
#include "ReplacementAlgorithm.hpp"
#include <list>
#include <unordered_map>

class LRU : public ReplacementAlgorithm {
    std::list<int> dll;
    std::unordered_map<int, std::list<int>::iterator> map;

public:
    void init(int) override { dll.clear(); map.clear(); }

    int selectVictim(const std::vector<int>& ram_owner) override {
        while (!dll.empty() && (dll.front() >= (int)ram_owner.size() || ram_owner[dll.front()] == -1)) {
            map.erase(dll.front());
            dll.pop_front();
        }
        int victim = dll.empty() ? 0 : dll.front();
        dll.pop_front();
        map.erase(victim);
        return victim;
    }

    void accessPage(int frame) override {
        if (map.count(frame)) dll.erase(map[frame]);
        dll.push_back(frame);
        map[frame] = --dll.end();
    }

    void pageRemoved(int frame) override {
        if (map.count(frame)) {
            dll.erase(map[frame]);
            map.erase(frame);
        }
    }

    std::string name() const override { return "LRU"; }
};