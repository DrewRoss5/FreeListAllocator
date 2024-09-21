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
    // test C string
    char* cStr = (char*) allocator.alloc(sizeof(char) * 20);
    allocator.dealloc(myPtr);
    allocator.dealloc(cStr);
}