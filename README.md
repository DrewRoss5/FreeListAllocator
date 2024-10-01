# FreeListAllocator
A simple object-oriented memory allocator implemented with a free list

# Getting started
- Ensure you have [GoogleTest](https://github.com/google/googletest/tree/main/googletest) installed
- Clone this repo
- Create a `build` directory in this repo's directory
- Run the following:
  - ```
    cd build
    cmake ..
    make
    ```
- The `build` directory will now contain a statically linked library (`libAlloc.a`)
- You can additionally ensure functionality by running the `Test` executable

# The `ListAllocator` Class:
The list allocator class is a heap memory allocator, it is initialized with a specific heap size and it dynamically allocates and deallocates memory. At the moment, allocated memory is returned as a `void*` simmilar to `malloc` in C. 
## Public Methods:
### ListAllocator(unsigned int size):
- Initalizes a ListAllocator, with a heap size of `size` bytes.
- Notes:
  - At least 4096 bytes (4K) is recommended for this
### void* alloc(unsigned int size):
  - Allocates a new block of memory in the heap of `size` bytes, and returns a `void` pointer to the block.
  - Notes:
    - This will return a null pointer if no block of the requested size could be found or made
### void dealloc(void* ptr):
  - Frees the memory associated with `ptr`, releasing the block to the free list.
  - Notes:
    - Throws a `std::bad_alloc` if `ptr` was not allocated by this object.
### void* realloc(void* ptr, unsigned int newSize):
  - Reallocates `ptr` to make `newSize` bytes large. This works for both growing and shrinking memory.
  - Notes:
    - If the memory pointed to by `ptr` is shrunk, the excess will simply be returned to the allocator as a new block.
    - If the memory is grown, the block will simply be extended in place, if enough contiguous space is available. Otherwise, the pointer will be reallocated to a new block. This means that using `realloc` may be more efficent than creating a
      new pointer and copying the data manually, however, it is not guranteed to be such.
    - Throws a `std::bad_alloc` if `ptr` was not allocated by this object.
### int getSize():
  - Returns the current number of available bytes.
### int getCapacity()
  - Returns the maximum capacity of the allocator.
### int getBlockCount():
  - Returns the number of free blocks in the list
  - Notes:
    - This currently only exists for debugging purposes, and will likely be removed in the future.
# Example
```cpp
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
```

