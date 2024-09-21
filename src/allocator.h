
struct Node{
    Node* next  {nullptr};
    Node* prev {nullptr};
    int size    {0};
};

class ListAllocator{
    public:
        ListAllocator(unsigned int size);
        ~ListAllocator();
        void* alloc(unsigned int size);
        void dealloc(void*);
        int getSize() {return size;}
        int getCapacity() {return capacity;}
    private:
        int capacity {0};
        int size     {0};
        Node* head {0};
        void* max;
        void* min;
        bool checkContinuity(Node* a, Node* b);
        Node* mergeNodes(Node* a, Node* b);
};
