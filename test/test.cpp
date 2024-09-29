#include <cstdlib>
#include <time.h>
#include <string>
#include <gtest/gtest.h>
#include "allocator.h"

TEST(AllocatorTests, SizeTests){
    ListAllocator allocator(4096);
    EXPECT_EQ(allocator.getSize(), 4096);
    int* ptr = (int*) allocator.alloc(sizeof(int) * 20);
    EXPECT_EQ(allocator.getSize(), 4096 - (sizeof(int) * 20));
    allocator.dealloc(ptr);
    EXPECT_EQ(allocator.getSize(), 4096);
}

// test that the size of the list grows appropriately as memory is returned
TEST(AllocatorTests, BlockCountTests){
    ListAllocator allocator(4096);
    char* str = (char*) allocator.alloc(sizeof(char) * 5);
    int* arr = (int*) allocator.alloc(sizeof(int) * 10);
    EXPECT_EQ(allocator.getBlockCount(), 1);
    allocator.dealloc(str);
    allocator.dealloc(arr);
    EXPECT_EQ(allocator.getBlockCount(), 2);
}

// test that blocks properly coalesce given that they are contiguous 
TEST(AllocatorTests, FragmentationTests){
    ListAllocator allocator(4096);
    char* str = (char*) allocator.alloc(sizeof(char) * 5);
    int* arr = (int*) allocator.alloc(sizeof(int) * 10);
    allocator.dealloc(str);
    allocator.dealloc(arr);
    allocator.coalesce();
    EXPECT_EQ(allocator.getBlockCount(), 1);
}

// test to ensure that out-of-bounds memory cannot be freed
TEST(AllocatorTests, OutOfBoundsTests){
    ListAllocator a(4096);
    ListAllocator b(4096);
    int* ptr1 = (int*) a.alloc(sizeof(int));
    int* ptr2 = (int*) b.alloc(sizeof(int));
    bool exceptionRaised = false;
    // try deallocating memory from another allocator
    try{
        a.dealloc(ptr2);
        b.dealloc(ptr1);
    }
    catch (std::bad_alloc){
        exceptionRaised = true;
    }
    EXPECT_TRUE(exceptionRaised);
    exceptionRaised = false;
    // try deallocating stack-allocated memory 
    try{
        int x = 5;
        int* y = &x;
        a.dealloc(y);
    }
    catch (std::bad_alloc){
        exceptionRaised = true;
    }
    EXPECT_TRUE(exceptionRaised);
    exceptionRaised = false;
    // try deallocating a nullptr
    try{
        b.dealloc(nullptr);
    }
    catch (std::bad_alloc){
        exceptionRaised = true;
    }
    EXPECT_TRUE(exceptionRaised);
    a.dealloc(ptr1);
    b.dealloc(ptr2);
}

// test to ensure memory is properly allocated when the size is unknown at compile time  
TEST(AllocatorTests, DynamicSizeTests){
    srand(time(nullptr));
    int size = 4096 + rand()%1024;
    ListAllocator allocator(size);
    EXPECT_EQ(allocator.getSize(), size);
    int* ptr = (int*) allocator.alloc(sizeof(int) * ((int) size / 4));
    allocator.dealloc(ptr);  
    EXPECT_EQ(allocator.getSize(), size);
}

// test to ensure that that memory reallocates properly (TODO: Add more edge cases to this)
TEST(AllocatorTests, RealloTests){
    ListAllocator allocator(4096);
    int* odds = (int*) allocator.alloc(sizeof(int) * 5);
    for (int i = 0; i < 5; i++){
        if (i % 2 == 0)
            odds[i] = i + 1;
        else 
            odds[i] = i + 2;
    }
    allocator.realloc(odds, (sizeof(int) * 10));
    for (int i = 5; i < 10; i++){
        if (i % 2 == 0)
            odds[i] = i + 1;
        else 
            odds[i] = i + 2;
    }
    EXPECT_EQ(allocator.getSize(), 4096 - (sizeof(int) * 10));
    for (int i = 0; i < 10; i++){
        if (i % 2 == 0)
            EXPECT_EQ(odds[i], i + 1);
        else 
            EXPECT_EQ(odds[i], i + 2);
    }
    allocator.dealloc(odds);
    EXPECT_EQ(allocator.getSize(), 4096);
}


int main(int argc, char** argv){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}