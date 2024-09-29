#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdexcept>
#include <exception>
#include "allocator.h"

ListAllocator::ListAllocator(unsigned int size){
    // create the map file and allocate the memory
    void* headPtr =  mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (headPtr == MAP_FAILED)
        throw std::bad_alloc();
    // create the head
    this->head = static_cast<Node*>(headPtr);
    this->head->next = nullptr;
    this->head->prev = nullptr;
    this->head->size = size;
    // update the class metadata
    this->size = size;
    this->capacity = size;
    // these variables are to be used for bounds checking 
    this->min = head;
    this->max = static_cast<void*>(((char*) head) + size);
}

ListAllocator::~ListAllocator(){
    if (munmap(this->head, this->size))
        std::cout << "Failed unmap.\nPotential memory leak" << std::endl;
}

// allocates a new void pointer of the specified size to the list
void* ListAllocator::alloc(unsigned int size){
    if (this->size < size)
        return nullptr;
    // find a suitable block of memory
    Node* prev = nullptr;
    Node* cur = this->head;
    while(cur){
        // allocate a new block, if this block is of the correct size
        if (cur->size >= size){
            // create a new block to take this one's place
            Node* newBlock = reinterpret_cast<Node*>(reinterpret_cast<unsigned char*>(cur) + sizeof(Node) + size);
            newBlock->size = cur->size - (size + sizeof(Node));
            newBlock->next = cur->next;
            newBlock->prev = cur->prev;
            // create the pointer to be returned
            cur->next = newBlock;
            cur->size = size;
            void* ptr = (void*) (reinterpret_cast<unsigned char*>(cur) + sizeof(Node));
            // remove cur from the list
            if (prev){
                newBlock->prev = prev;
                prev->next = newBlock;
            }
            else
                this->head = newBlock;
            this->size -= size + sizeof(Node);
            return ptr;
        }
        // continue traversing the list
        prev = cur;
        cur = cur->next;   
    }
    // no block could be found, return a nullptr
    return nullptr;
}

// deallocates a pointer and returns the memory to the free list
// throws a std::bad_alloc if the pointer was not allocated by this allocator
void ListAllocator::dealloc(void* ptr){
    if (ptr > max || ptr < min)
        throw std::bad_alloc();
    // convert the pointer into a node
    Node* newBlock = reinterpret_cast<Node*>(((char*) ptr) - sizeof(Node));
    this->size += newBlock->size + sizeof(Node);
    // return the block
    this->insertNode(newBlock);
}

// reallocates the given pointer, extending it in place if possible, or copying it to a new address if not possible. Returns a nullptr if no appropriate block can be found
void* ListAllocator::realloc(void* ptr, unsigned int newSize){ 
    if (ptr > max || ptr < min)
        throw std::bad_alloc();
    // convert the pointer into a node
    Node* ptrBlock = reinterpret_cast<Node*>(((char*) ptr) - sizeof(Node));
    // shrink the block and return the excess if the size is smaller than the current size
    if (ptrBlock->size > newSize){
        int dif = ptrBlock->size - newSize;
        Node* newBlock = reinterpret_cast<Node*> (((char*) ptr) + newSize + sizeof(Node));
        newBlock->size = dif;
        ptrBlock->size = newSize;
        this->size += dif;
        return ptr;
    }
    // find a contiguous block if possible
    Node* cur = this->head;
    Node* newBlock = nullptr;
    while (cur){
        if (checkContinuity(ptrBlock, cur) && (ptrBlock->size + cur->size) >= newSize){
            newBlock = cur;
            break;
        }
    }
    // contiguous block found, simply increase the size of the given black
    if (newBlock){
        // move the newNode forward
        Node* next = newBlock->next;
        Node* prev = newBlock->prev;
        newBlock = reinterpret_cast<Node*>((void*) (reinterpret_cast<char*>(newBlock) + (newSize - ptrBlock->size )));
        newBlock->prev = prev;
        newBlock->next = next;
        newBlock->size = newSize - ptrBlock->size;
        if (!newBlock->prev)
            this->head = newBlock;    
        // resize and return this block
        this->size -= newSize - ptrBlock->size;
        ptrBlock->size = newSize;
        return (void*) (reinterpret_cast<unsigned char*>(ptrBlock) + sizeof(Node));
    }
    // no contiguous block could be found
    void* newPtr = this->alloc(newSize);
    memcpy(newPtr, ptr, ptrBlock->size);
    this->dealloc(ptr);
    return newPtr;
}

// explicitly coalesces the list, merging all contiguous nodes. Mostly for debugging purposes, or very particular performance requirements
void ListAllocator::coalesce(){
    Node* prev = this->head;
    Node* curr = prev->next;
    while (curr){
        if (checkContinuity(prev, curr))
            curr = mergeNodes(prev, curr);
        curr = curr->next;    
    }
}

// checks if two node pointers are contiguous in memory, and returns true if they are
bool ListAllocator::checkContinuity(Node* a, Node* b) const {
    return (void*) ((char*) b - (a->size + sizeof(Node))) == a;
}

// merges two contiguous nodes into one (assumes that b comes after a)
Node* ListAllocator::mergeNodes(Node* a, Node* b){
    Node* newBlock = a; 
    newBlock->size += b->size;
    newBlock->next = b->next;
    if (b->next)
        b->next->prev = newBlock;
    return newBlock;
}

// inserts a new node, in order, into the free list, coallescing the new node if it's contiguous with it's neighbor
void ListAllocator::insertNode(Node* block){
        // find where the new node belongs in memory
    if (block < this->head){
        if (checkContinuity(block, this->head)){
            Node* tmp = this->head->next;
            this->head = mergeNodes(block, this->head);
            if (tmp)
                tmp->prev = this->head;
        }
        else{
            Node* tmp = this->head;
            block->next = tmp;
            tmp->prev = block;
            this->head = block;
        }
    }
    else{
        Node* prev = this->head;
        while (prev <  block && prev->next)
            prev = prev->next;
        prev = prev->prev; // get the last node that was previous to this one
        // check if  the two nodes are contiguous in memory and merge them if so
        if (prev->next && checkContinuity(block, prev->next))
            block = mergeNodes(block, prev->next);
        prev->next = block;
        if (prev->next)
            prev->next->prev = block;
    }   
}

// this is for debugging purposes and will be removed
int ListAllocator::getBlockCount() const {
    int count = 0;
    Node* cur = this->head;
    while (cur){
        cur = cur->next;
        count++; 
    }
    return count;
}