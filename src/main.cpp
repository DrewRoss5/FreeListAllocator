#include <iostream>
#include "allocator.h"

int main(){
    int x = 10;
    int* y = &x;
    ListAllocator allocator(4096);
    // test arrays
    int* myPtr = (int*) allocator.alloc(sizeof(int) * 10);
    for (int i = 0; i < 10; i++)
        myPtr[i] = i + 1;
    int* ptr2 = (int*) allocator.alloc(sizeof(int) * 20);
    std::cout << "Node count: " << allocator.getBlockCount() << std::endl;
    allocator.dealloc(ptr2);
    std::cout << "Node count: " << allocator.getBlockCount() << std::endl;
    allocator.dealloc(myPtr);


    
    

}