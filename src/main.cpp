#include <iostream>
#include "allocator.h"

int main(){
    ListAllocator allocator(4096);
    // get the size of the array
    int size;
    std::cout << "Array size: ";
    std::cin >> size;
    int* myPtr = (int*) allocator.alloc(sizeof(int) * size);
    for (int i = 0; i < size; i++){
        myPtr[i] = i + 1;
        std::cout << myPtr[i] << std::endl;
    }
    allocator.dealloc(myPtr);
}
