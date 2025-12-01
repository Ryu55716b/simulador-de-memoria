cmake_minimum_required(VERSION 3.16)
project(MemorySimulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(simulator
    src/main.cpp
    src/MemoryManager.cpp
    src/FIFO.cpp
    src/LRU.cpp
    src/Clock.cpp
)

target_include_directories(simulator PRIVATE src)