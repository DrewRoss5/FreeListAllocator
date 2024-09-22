project(MemoryAllocator)
CMAKE_MINIMUM_REQUIRED(VERSION 3.28)

find_package(GTest)
include(GoogleTest)
add_executable(Test test/test.cpp src/allocator.h src/allocator.cpp)
target_link_libraries(Test PRIVATE GTest::gtest)
target_include_directories(Test PUBLIC src)