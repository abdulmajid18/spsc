#include <gtest/gtest.h>
#include "../../include/spsc/circular_buffer.hpp"

class CircularBufferTest : public ::testing::Test {
protected:
    static constexpr size_t BUFFER_SIZE = 8;
    spsc::CircularBuffer<int, BUFFER_SIZE> buffer;
};

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
TEST_F(CircularBufferTest, PushPopMultipleElements) {
    EXPECT_TRUE(buffer.push(1));
    EXPECT_TRUE(buffer.push(2));
    EXPECT_TRUE(buffer.push(3));
    EXPECT_TRUE(buffer.push(4));

    EXPECT_EQ(buffer.pop(), 1);
    EXPECT_EQ(buffer.pop(), 2);
    EXPECT_EQ(buffer.pop(), 3);
    EXPECT_EQ(buffer.pop(), 4);
}

TEST_F(CircularBufferTest, PushToFullBuffer) {
    // Fill the buffer
    for (size_t i = 0; i < BUFFER_SIZE; ++i) {
        EXPECT_TRUE(buffer.push(i));
    }
    
    // Attempt to push to full buffer
    EXPECT_FALSE(buffer.push(BUFFER_SIZE));
    
    // Verify buffer contents remain unchanged
    for (size_t i = 0; i < BUFFER_SIZE; ++i) {
        EXPECT_EQ(buffer.pop(), i);
    }
}

TEST_F(CircularBufferTest, PopFromEmptyBuffer) {
    EXPECT_TRUE(buffer.is_empty());
    EXPECT_THROW(buffer.pop(), std::runtime_error);
}

TEST_F(CircularBufferTest, WrapAroundBehavior) {
    // Fill the buffer to capacity
    for (size_t i = 0; i < BUFFER_SIZE; ++i) {
        EXPECT_TRUE(buffer.push(i));
    }

    // Remove half of the elements
    for (size_t i = 0; i < BUFFER_SIZE / 2; ++i) {
        EXPECT_EQ(buffer.pop(), i);
    }

    // Add new elements, causing wrap-around
    for (size_t i = 0; i < BUFFER_SIZE / 2; ++i) {
        EXPECT_TRUE(buffer.push(i + BUFFER_SIZE));
    }

    // Verify all elements in the correct order
    for (size_t i = BUFFER_SIZE / 2; i < BUFFER_SIZE; ++i) {
        EXPECT_EQ(buffer.pop(), i);
    }
    for (size_t i = 0; i < BUFFER_SIZE / 2; ++i) {
        EXPECT_EQ(buffer.pop(), i + BUFFER_SIZE);
    }
}

// TEST_F(CircularBufferTest, MaintainOrderAfterMultiplePushPop) {
//     EXPECT_TRUE(buffer.push(1));
//     EXPECT_TRUE(buffer.push(2));
//     EXPECT_TRUE(buffer.push(3));
//     EXPECT_TRUE(buffer.push(4));

//     EXPECT_EQ(buffer.pop(), 1);
//     EXPECT_EQ(buffer.pop(), 2);

//     EXPECT_TRUE(buffer.push(5));
//     EXPECT_TRUE(buffer.push(6));

//     EXPECT_EQ(buffer.pop(), 3);
//     EXPECT_EQ(buffer.pop(), 4);
//     EXPECT_EQ(buffer.pop(), 5);
//     EXPECT_EQ(buffer.pop(), 6);
// }

// TEST_F(CircularBufferTest, PushPopDifferentDataTypes) {
//     spsc::CircularBuffer<std::variant<int, double, std::string>, BUFFER_SIZE> multi_type_buffer;

//     EXPECT_TRUE(multi_type_buffer.push(42));
//     EXPECT_TRUE(multi_type_buffer.push(3.14));
//     EXPECT_TRUE(multi_type_buffer.push(std::string("test")));

//     EXPECT_EQ(std::get<int>(multi_type_buffer.pop()), 42);
//     EXPECT_EQ(std::get<double>(multi_type_buffer.pop()), 3.14);
//     EXPECT_EQ(std::get<std::string>(multi_type_buffer.pop()), "test");
// }

// TEST_F(CircularBufferTest, UpdateSizeAfterPushPop) {
//     EXPECT_EQ(buffer.size(), 0);
    
//     EXPECT_TRUE(buffer.push(1));
//     EXPECT_EQ(buffer.size(), 1);
    
//     EXPECT_TRUE(buffer.push(2));
//     EXPECT_EQ(buffer.size(), 2);
    
//     EXPECT_EQ(buffer.pop(), 1);
//     EXPECT_EQ(buffer.size(), 1);
    
//     EXPECT_EQ(buffer.pop(), 2);
//     EXPECT_EQ(buffer.size(), 0);
// }

// TEST_F(CircularBufferTest, AlternatingPushPop) {
//     EXPECT_TRUE(buffer.push(1));
//     EXPECT_EQ(buffer.pop(), 1);
    
//     EXPECT_TRUE(buffer.push(2));
//     EXPECT_EQ(buffer.pop(), 2);
    
//     EXPECT_TRUE(buffer.push(3));
//     EXPECT_TRUE(buffer.push(4));
//     EXPECT_EQ(buffer.pop(), 3);
    
//     EXPECT_TRUE(buffer.push(5));
//     EXPECT_EQ(buffer.pop(), 4);
//     EXPECT_EQ(buffer.pop(), 5);
    
//     EXPECT_TRUE(buffer.is_empty());
// }

// TEST_F(CircularBufferTest, PushMaxIntValue) {
//     EXPECT_TRUE(buffer.push(std::numeric_limits<int>::max()));
//     EXPECT_TRUE(buffer.push(1));
    
//     EXPECT_EQ(buffer.pop(), std::numeric_limits<int>::max());
//     EXPECT_EQ(buffer.pop(), 1);
// }

// TEST_F(CircularBufferTest, ResetStateAfterClear) {
//     EXPECT_TRUE(buffer.push(1));
//     EXPECT_TRUE(buffer.push(2));
    
//     EXPECT_EQ(buffer.pop(), 1);
//     EXPECT_EQ(buffer.pop(), 2);
    
//     // At this point, the buffer should be empty
//     EXPECT_TRUE(buffer.is_empty());
    
//     // Push new elements to verify the buffer state has been reset
//     EXPECT_TRUE(buffer.push(3));
//     EXPECT_TRUE(buffer.push(4));
    
//     EXPECT_EQ(buffer.pop(), 3);
//     EXPECT_EQ(buffer.pop(), 4);
// }