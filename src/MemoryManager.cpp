#include "MemoryManager.hpp"
#include "FIFO.hpp"
#include "LRU.hpp"
#include "Clock.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>

MemoryManager::MemoryManager(const std::string& file) {
    if (!loadConfig(file)) {
        std::cerr << "Error: No se pudo cargar config.ini\n";
        std::exit(1);
    }

    ram_owner.assign(ram_frames, -1);
    ram_page_in_ram.assign(ram_frames, -1);
    swap_owner.assign(swap_frames, -1);

    if (algo_name == "fifo") algo = std::make_unique<FIFO>();
    else if (algo_name == "lru") algo = std::make_unique<LRU>();
    else algo = std::make_unique<Clock>();

    algo->init(ram_frames);
    if (tlb_enabled) tlb.resize(tlb_size);

    std::cout << "SIMULADOR INICIADO - " << algo->name() << " - RAM: " << ram_frames
              << " marcos - TLB: " << (tlb_enabled ? "Activada" : "Desactivada") << "\n\n";
}

MemoryManager::~MemoryManager() {
    for (auto& p : active) delete p.second;
}

bool MemoryManager::loadConfig(const std::string& f) {
    std::ifstream file(f);
    if (!file) return false;
    std::string line, key, val;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == ';' || line[0] == '#') continue;
        std::istringstream iss(line);
        if (iss >> key >> val) {
            if (key == "ram_size_kb") ram_kb = std::stoi(val);
            else if (key == "swap_size_kb") swap_kb = std::stoi(val);
            else if (key == "page_size_kb") page_kb = std::stoi(val);
            else if (key == "replacement_algorithm") algo_name = val;
            else if (key == "enable_tlb") tlb_enabled = (val == "true");
            else if (key == "tlb_size") tlb_size = std::stoi(val);
        }
    }
    ram_frames = ram_kb / page_kb;
    swap_frames = swap_kb / page_kb;
    return ram_frames > 0;
}

void MemoryManager::runSimulation() {
    for (int step = 1; step <= 1200; ++step) {
        if (rand() % 100 < 18) createProcess();
        for (int i = 0; i < 6; ++i) randomAccess();
        if (rand() % 100 < 8 && !active.empty()) killRandomProcess();

        if (step % 100 == 0) {
            std::cout << "\n=== PASO " << step << " ===\n";
            printMemoryMap();
        }

        // Nueva: Mostrar tabla de paginas de un proceso aleatorio cada 300 pasos
        if (step % 300 == 0 && !active.empty()) {
            auto it = active.begin();
            std::advance(it, rand() % active.size());
            printPageTable(it->first);
        }
    }
    printStats();
}

void MemoryManager::createProcess() {
    int size_kb = 256 + (rand() % 15) * 256;
    int pages = (size_kb + page_kb - 1) / page_kb;
    Process* p = new Process(next_pid++, size_kb, pages, 100 + rand() % 150);
    active[p->pid] = p;

    int loaded = 0;
    for (int i = 0; i < ram_frames && loaded < pages; ++i) {
        if (ram_owner[i] == -1) {
            ram_owner[i] = p->pid;
            ram_page_in_ram[i] = loaded;
            p->page_table[loaded].frame = i;
            p->page_table[loaded].present = true;
            loaded++;
        }
    }
    std::cout << "Creado P" << p->pid << " (" << pages << " paginas)\n";
}

void MemoryManager::killRandomProcess() {
    if (active.empty()) return;
    auto it = active.begin();
    std::advance(it, rand() % active.size());
    Process* p = it->second;
    std::cout << "Terminando P" << p->pid << "\n";

    for (int i = 0; i < ram_frames; ++i) {
        if (ram_owner[i] == p->pid) {
            algo->pageRemoved(i);
            ram_owner[i] = ram_page_in_ram[i] = -1;
        }
    }
    for (int i = 0; i < swap_frames; ++i)
        if (swap_owner[i] == p->pid) swap_owner[i] = -1;

    active.erase(it);
    delete p;
}

void MemoryManager::randomAccess() {
    if (active.empty()) return;
    accesses++;
    auto it = active.begin();
    std::advance(it, rand() % active.size());
    Process* p = it->second;
    int page = rand() % p->pages_needed;

    int frame = lookupTLB(p->pid, page);
    if (frame != -1) { tlb_hits++; algo->accessPage(frame); return; }

    if (p->page_table[page].present) {
        frame = p->page_table[page].frame;
        updateTLB(p->pid, page, frame);
        algo->accessPage(frame);
    } else {
        pageFault(p->pid, page);
    }
}

void MemoryManager::pageFault(int pid, int page) {
    faults++;
    Process* p = active[pid];
    int victim = algo->selectVictim(ram_owner);

    if (ram_owner[victim] != -1) {
        int old_pid = ram_owner[victim];
        int old_page = ram_page_in_ram[victim];
        active[old_pid]->page_table[old_page].present = false;
        active[old_pid]->page_table[old_page].frame = -1;

        int swap_slot = findFreeSwapFrame();
        if (swap_slot != -1) {
            swap_owner[swap_slot] = old_pid;
            active[old_pid]->page_table[old_page].swap_frame = swap_slot;
            swap_out++;
        }
        algo->pageRemoved(victim);
    }

    if (p->page_table[page].swap_frame != -1) {
        swap_owner[p->page_table[page].swap_frame] = -1;
        swap_in++;
    }

    ram_owner[victim] = pid;
    ram_page_in_ram[victim] = page;
    p->page_table[page].frame = victim;
    p->page_table[page].present = true;
    p->page_table[page].swap_frame = -1;
    updateTLB(pid, page, victim);
    algo->accessPage(victim);
}

int MemoryManager::findFreeSwapFrame() {
    for (int i = 0; i < swap_frames; ++i)
        if (swap_owner[i] == -1) return i;
    return -1;
}

int MemoryManager::lookupTLB(int pid, int page) {
    for (const auto& e : tlb)
        if (e.pid == pid && e.page == page) return e.frame;
    return -1;
}

void MemoryManager::updateTLB(int pid, int page, int frame) {
    if (!tlb_enabled) return;
    for (auto& e : tlb) {
        if (e.pid == pid && e.page == page) { e.frame = frame; return; }
    }
    tlb.push_back({pid, page, frame});
    if (tlb.size() > tlb_size) tlb.erase(tlb.begin());
}

void MemoryManager::printMemoryMap() const {
    std::cout << "RAM: ";
    for (int i = 0; i < ram_frames; ++i) {
        if (ram_owner[i] == -1) std::cout << "[----] ";
        else std::cout << "P" << ram_owner[i] << ".p" << ram_page_in_ram[i] << " ";
    }
    std::cout << "\n";
}

void MemoryManager::printStats() const {
    std::cout << "\n================================================\n";
    std::cout << "          ESTADISTICAS FINALES                \n";
    std::cout << "================================================\n";
    std::cout << "Accesos: " << accesses << " | Fallos: " << faults
              << " (" << (accesses ? 100.0 * faults / accesses : 0) << "%)\n";
    std::cout << "TLB Hits: " << tlb_hits << " (" << (accesses ? 100.0 * tlb_hits / accesses : 0) << "%)\n";
    std::cout << "Swap Out/In: " << swap_out << " / " << swap_in << "\n";
}

void MemoryManager::printPageTable(int pid) const {
    auto it = active.find(pid);
    if (it == active.end()) {
        std::cout << "Proceso P" << pid << " no existe o fue terminado.\n";
        return;
    }

    const Process* p = it->second;
    std::cout << "\n";
    std::cout << "================================================\n";
    std::cout << "      TABLA DE PAGINAS - PROCESO P" << std::setw(3) << pid << "      \n";
    std::cout << "================================================\n";
    std::cout << " Pagina | Presente | Marco RAM | Slot Swap \n";
    std::cout << "--------|----------|-----------|-----------\n";

    for (int i = 0; i < p->pages_needed; ++i) {
        const PageEntry& entry = p->page_table[i];
        std::cout << std::setw(6) << i << "  | ";
        std::cout << (entry.present ? "   Si    | " : "   No    | ");
        if (entry.frame != -1)
            std::cout << std::setw(6) << entry.frame << "    | ";
        else
            std::cout << "   -      | ";
        if (entry.swap_frame != -1)
            std::cout << std::setw(6) << entry.swap_frame;
        else
            std::cout << "   -   ";
        std::cout << "\n";
    }
    std::cout << "\n";
}