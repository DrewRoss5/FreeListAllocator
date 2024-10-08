
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
        void dealloc(void* ptr);
        void* realloc(void* ptr, unsigned int size);
        void coalesce(); 
        int getSize() const {return size;}
        int getCapacity() const {return capacity;} 
        int getBlockCount() const;
    private:
        int capacity {0};
        int size     {0};
        Node* head {0};
        void* max {nullptr};
        void* min {nullptr};
        bool checkContinuity(Node* a, Node* b) const;
        Node* mergeNodes(Node* a, Node* b);
        void insertNode(Node* node);
};
