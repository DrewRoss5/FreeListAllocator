
struct Node{
    Node* next  {nullptr};
    int size    {0};
};

class ListAllocator{
    public:
        ListAllocator(unsigned int size);
        ~ListAllocator();
        void* alloc(unsigned int size);
        void dealloc(void*);
        int get_size() {return size;}
        int get_capacity() {return capacity;}
    private:
        int capacity {0};
        int size     {0};
        Node* head {0};
        void* max;
        void* min;
};
