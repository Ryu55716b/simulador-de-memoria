#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include "Process.hpp"
#include "ReplacementAlgorithm.hpp"

class MemoryManager {
private:
    // Configuración
    int ram_kb{}, swap_kb{}, page_kb{};
    int ram_frames{}, swap_frames{};
    std::string algo_name{};
    bool tlb_enabled{false};
    int tlb_size{8};

    // Estado de memoria
    std::vector<int> ram_owner;         // PID dueño del marco (-1 = libre)
    std::vector<int> ram_page_in_ram;   // número de página en ese marco
    std::vector<int> swap_owner;        // PID en swap

    // Procesos
    std::unordered_map<int, Process*> active;
    int next_pid{1};

    // Algoritmo de reemplazo
    std::unique_ptr<ReplacementAlgorithm> algo;

    // TLB
    struct TLBEntry { int pid{-1}, page{-1}, frame{-1}; };
    std::vector<TLBEntry> tlb;

    // Estadísticas
    long long accesses{0}, faults{0}, tlb_hits{0}, swap_out{0}, swap_in{0};

public:
    MemoryManager(const std::string& config_file = "config.ini");
    ~MemoryManager();
    void runSimulation();

    // ¡NUEVA FUNCIÓN PÚBLICA PARA MOSTRAR TABLA DE PÁGINAS!
    void printPageTable(int pid) const;

private:
    bool loadConfig(const std::string& file);
    void createProcess();
    void killRandomProcess();
    void randomAccess();
    int findFreeSwapFrame();
    void pageFault(int pid, int page);
    int lookupTLB(int pid, int page);
    void updateTLB(int pid, int page, int frame);
    void printMemoryMap() const;
    void printStats() const;
};