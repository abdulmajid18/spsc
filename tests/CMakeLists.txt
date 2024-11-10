cmake_minimum_required(VERSION 3.10)

set(This Example)

project(CircularBufferTest)

# Enable C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable testing
enable_testing()

# Find GTest package
find_package(GTest REQUIRED)

# Add test executable
add_executable(buffer_test circular_buffer_test.cpp)

# Link against GTest and pthread
target_link_libraries(buffer_test
    GTest::GTest
    GTest::Main
    pthread
)

# Add test to CTest
add_test(NAME buffer_test COMMAND buffer_test)