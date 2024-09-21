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
    this->head->size = size;
    // update the class metadata
    this->size = size;
    this->capacity = size;
    // these variables are to be used for bounds checking 
    this->min = head;
    this->max = static_cast<void*>(((char*) head) + size);
}

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
            if (prev)
                prev->next = newBlock;
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


void ListAllocator::dealloc(void* ptr){
    if (ptr > max || ptr < min)
        throw std::bad_alloc();
    // convert the pointer into a node
    Node* newNode = reinterpret_cast<Node*>(ptr - sizeof(Node));
    // find where the new node belongs in memory
    if (newNode < this->head){
        Node* tmp = this->head;
        newNode->next = tmp;
        this->head = newNode;
    }
    else{
        Node* prev = this->head;
        while (prev <  newNode && prev->next)
            prev = prev->next;
        Node* tmp = prev->next;
        prev->next = newNode;
        newNode->next= tmp;
    }    
}

ListAllocator::~ListAllocator(){
    if (munmap(this->head, this->size)){
        std::cout << "Failed unmap" << std::endl;
    }
}