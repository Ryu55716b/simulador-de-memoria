#pragma once
#include "ReplacementAlgorithm.hpp"
#include <vector>

class Clock : public ReplacementAlgorithm {
    int hand = 0;
    std::vector<bool> ref_bit;

public:
    void init(int total_frames) override {
        ref_bit.assign(total_frames, false);
        hand = 0;
    }

    int selectVictim(const std::vector<int>& ram_owner) override {
        while (true) {
            if (!ref_bit[hand]) {
                int victim = hand;
                hand = (hand + 1) % ram_owner.size();
                return victim;
            }
            ref_bit[hand] = false;
            hand = (hand + 1) % ram_owner.size();
        }
    }

    void accessPage(int frame) override {
        if (frame < (int)ref_bit.size())
            ref_bit[frame] = true;
    }

    void pageRemoved(int frame) override {
        if (frame < (int)ref_bit.size())
            ref_bit[frame] = false;
    }

    std::string name() const override { return "Reloj (Clock)"; }
};