#pragma once
#include <vector>
#include <string>

class ReplacementAlgorithm {
public:
    virtual ~ReplacementAlgorithm() = default;
    virtual void init(int total_frames) {}
    virtual int selectVictim(const std::vector<int>& ram_owner) = 0;
    virtual void accessPage(int frame) = 0;
    virtual void pageRemoved(int frame) = 0;
    virtual std::string name() const = 0;
};