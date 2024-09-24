#include <iostream>
#include <stdlib.h>
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
            this->size -= size;
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
    Node* newNode = reinterpret_cast<Node*>(ptr - sizeof(Node));
    this->size += newNode->size;
    // find where the new node belongs in memory
    if (newNode < this->head){
        if (checkContinuity(newNode, this->head)){
            Node* tmp = this->head->next;
            this->head = mergeNodes(newNode, this->head);
            if (tmp)
                tmp->prev = this->head;
        }
        else{
            Node* tmp = this->head;
            newNode->next = tmp;
            tmp->prev = newNode;
            this->head = newNode;
        }
    }
    else{
        Node* prev = this->head;
        while (prev <  newNode && prev->next)
            prev = prev->next;
        prev = prev->prev; // get the last node that was previous to this one
        // check if  the two nodes are contiguous in memory and merge them if so
        if (prev->next && checkContinuity(newNode, prev->next))
            newNode = mergeNodes(newNode, prev->next);
        prev->next = newNode;
        if (prev->next)
            prev->next->prev = newNode;
    }   

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
    Node* newNode = a; 
    newNode->size += b->size;
    newNode->next = b->next;
    if (b->next)
        b->next->prev = newNode;
    return newNode;
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