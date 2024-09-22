#include <iostream>
#include <cstring>
#include "allocator.h"

struct Point{
    int x;
    int y;
};

int main(){
    int x = 10;
    int* y = &x;
    ListAllocator allocator(4096);
    // test arrays
    int* myPtr = (int*) allocator.alloc(sizeof(int) * 10);
    for (int i = 0; i < 10; i++)
        myPtr[i] = i + 1;
    // run with c-strings
    char* hello = (char*) allocator.alloc(sizeof(char) * 14);
    std::strcpy(hello, "Hello, World!");
    std::cout << hello << std::endl;
    // allocate a struct 
    Point* coolPlace = (Point*) allocator.alloc(sizeof(Point));
    coolPlace->x = 37;
    coolPlace->y = 115;
    // ensure deallocation works (node count to check coallescing)
    allocator.dealloc(coolPlace);
    std::cout << "Block count: " << allocator.getBlockCount() << std::endl;
    allocator.dealloc(hello);
    std::cout << "Block count: " << allocator.getBlockCount() << std::endl;
    allocator.dealloc(myPtr);
    std::cout << "Block count: " << allocator.getBlockCount() << std::endl;

}