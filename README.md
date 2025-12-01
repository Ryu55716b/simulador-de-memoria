# Simulador de Memoria Virtual con Paginación y Swap

## i. Nombres de los integrantes
Mendez Guerrero Pablo Daniel
a2223330178
Ortega Resendiz Luis Fernando
a2183330150
Sanchez Morales Jesus
a2223339020
Vergara Gonzalez Magnus Henrich
a2143222011


## ii. Cómo compilar y ejecutar

### Forma más fácil
1. Descargar simulador.exe y config.ini desde Github  
2. Ponerlos en la misma carpeta  
3. Hacer doble clic en simulador.exe  
→ Arranca solo y muestra toda la simulación

Para probar otro algoritmo:  
Abrir config.ini → cambiar la línea  
replacement_algorithm clock  
por fifo o lru → guardar → volver a abrir el .exe

### En VS Code  
Abrir la carpeta del proyecto → presionar F5 → se compila y corre automáticamente

### Desde terminal
g++ -std=c++17 src/*.cpp -o simulador.exe  
./simulador.exe

## iii. Diseño del programa

- RAM: dos vectores (ram_owner y ram_page_in_ram) para saber en tiempo constante quién ocupa cada marco.  
- Swap: un solo vector swap_owner para buscar rápido slots libres.  
- Cada proceso tiene su propia tabla de páginas (vector<PageEntry>) con los campos present, frame y swap_frame.  
- Los procesos activos están guardados en un unordered_map<PID, Process*> para buscarlos en O(1).  
- TLB: un vector simple con política FIFO (el más viejo sale primero).  
- Algoritmos de reemplazo: hice una clase abstracta ReplacementAlgorithm y tres clases que heredan: FIFO, LRU y Clock.  
  El programa elige cuál usar leyendo el config.ini, así no hay que tocar código para cambiarlo.

Todo está hecho en C++17, es puro código de consola y corre en cualquier Windows sin instalar nada.
