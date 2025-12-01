#pragma once
#include <vector>

struct PageEntry {
    int frame = -1;        // -1 = no está en RAM
    int swap_frame = -1;   // -1 = no está en swap
    bool present = false;
};

class Process {
public:
    int pid;
    int size_kb;
    int pages_needed;
    int lifetime;
    std::vector<PageEntry> page_table;

    Process(int id, int size, int pages, int life)
        : pid(id), size_kb(size), pages_needed(pages), lifetime(life) {
        page_table.resize(pages);
    }
};