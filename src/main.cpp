#include "MemoryManager.hpp"
#include <iostream>
#include <cstdlib>
#include <ctime>

int main() {
    std::srand(std::time(nullptr));  // ÚNICA vez en todo el programa

    std::cout << "==================================================\n";
    std::cout << "    SIMULADOR DE GESTION DE MEMORIA - SO 2025     \n";
    std::cout << "       Paginacion + Swap + TLB + FIFO/LRU/CLOCK    \n";
    std::cout << "==================================================\n\n";

    MemoryManager simulador("config.ini");
    simulador.runSimulation();

    std::cout << "\nSimulacion terminada. Presiona Enter para cerrar...\n";
    std::cin.get();
    return 0;
}