project(MemoryAllocator)
CMAKE_MINIMUM_REQUIRED(VERSION 3.28)

add_executable(Test src/main.cpp src/allocator.h src/allocator.cpp)