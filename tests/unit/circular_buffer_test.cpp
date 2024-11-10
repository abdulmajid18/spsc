#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <chrono>
#include "../../include/spsc/circular_buffer.hpp"


class CircularBufferTest : public ::testing::Test {
protected:
    static constexpr size_t BUFFER_SIZE = 8;  // Power of 2 size
    spsc::CircularBuffer<int, BUFFER_SIZE> buffer;
};

// Basic Operations Tests
TEST_F(CircularBufferTest, BasicPushPop) {
    EXPECT_TRUE(buffer.push(1));
    EXPECT_TRUE(buffer.push(2));
    
    EXPECT_EQ(buffer.pop(), 1);
    EXPECT_EQ(buffer.pop(), 2);
}

TEST_F(CircularBufferTest, EmptyBufferOperations) {
    EXPECT_TRUE(buffer.is_empty());
    EXPECT_FALSE(buffer.is_full());
    EXPECT_EQ(buffer.size(), 0);
    
    EXPECT_THROW(buffer.pop(), std::runtime_error);
    EXPECT_THROW(buffer.peek_oldest(), std::runtime_error);
    EXPECT_THROW(buffer.peek_latest(), std::runtime_error);
}

TEST_F(CircularBufferTest, FullBufferOperations) {
    // Fill the buffer
    for (int i = 0; i < BUFFER_SIZE; ++i) {
        EXPECT_TRUE(buffer.push(std::move(i)));
    }
    
    EXPECT_TRUE(buffer.is_full());
    EXPECT_FALSE(buffer.is_empty());
    EXPECT_EQ(buffer.size(), BUFFER_SIZE);
    
    // Attempt to push to full buffer should fail
    EXPECT_FALSE(buffer.push(100));
}

TEST_F(CircularBufferTest, WrapAroundBehavior) {
    // Fill buffer
    std::cout << "Size Before pushing: " << buffer.size() << std::endl;
    for (int i = 0; i < BUFFER_SIZE; ++i) {
        EXPECT_TRUE(buffer.push(std::move(i)));
    }
    
    std::cout << "Size After pushing: " << buffer.size() << std::endl;
    EXPECT_EQ(buffer.peek_latest(), 7);

    // Remove half
    for (int i = 0; i < BUFFER_SIZE / 2; ++i) {
        EXPECT_EQ(buffer.pop(), i); 
    }


    std::cout << "Size After Poping BUFFER_SIZE/2: " << buffer.size() << std::endl;
    EXPECT_EQ(buffer.peek_oldest(), 4);
    
    // Add new elements, causing wraparound
    for (int i = 0; i < BUFFER_SIZE/2; ++i) {
        EXPECT_TRUE(buffer.push(std::move(i + BUFFER_SIZE)));
    }

    EXPECT_EQ(buffer.peek_latest(), 11);
    
    // Verify all elements
    for (int i = BUFFER_SIZE/2; i < BUFFER_SIZE; ++i) {
        EXPECT_EQ(buffer.pop(), i);
    }

}

// Peek Operations Tests
TEST_F(CircularBufferTest, PeekOperations) {
    buffer.push(1);
    buffer.push(2);
    
    EXPECT_EQ(buffer.peek_oldest(), 1);
    EXPECT_EQ(buffer.peek_latest(), 2);
    
    // Verify peeks didn't remove elements
    EXPECT_EQ(buffer.pop(), 1);
    EXPECT_EQ(buffer.pop(), 2);
}

TEST_F(CircularBufferTest, MaintainOrderAfterMultiplePushPop) {
    EXPECT_TRUE(buffer.push(1));
    EXPECT_TRUE(buffer.push(2));
    EXPECT_TRUE(buffer.push(3));
    EXPECT_TRUE(buffer.push(4));

    EXPECT_EQ(buffer.pop(), 1);
    EXPECT_EQ(buffer.pop(), 2);

    EXPECT_TRUE(buffer.push(5));
    EXPECT_TRUE(buffer.push(6));

    EXPECT_EQ(buffer.pop(), 3);
    EXPECT_EQ(buffer.pop(), 4);
    EXPECT_EQ(buffer.pop(), 5);
    EXPECT_EQ(buffer.pop(), 6);
}

TEST_F(CircularBufferTest, PushPopDifferentDataTypes) {
    spsc::CircularBuffer<std::variant<int, double, std::string>, BUFFER_SIZE> multi_type_buffer;

    EXPECT_TRUE(multi_type_buffer.push(42));
    EXPECT_TRUE(multi_type_buffer.push(3.14));
    EXPECT_TRUE(multi_type_buffer.push(std::string("test")));

    EXPECT_EQ(std::get<int>(multi_type_buffer.pop()), 42);
    EXPECT_EQ(std::get<double>(multi_type_buffer.pop()), 3.14);
    EXPECT_EQ(std::get<std::string>(multi_type_buffer.pop()), "test");
}

TEST_F(CircularBufferTest, PushMaxIntValue) {
    EXPECT_TRUE(buffer.push(std::numeric_limits<int>::max()));
    EXPECT_TRUE(buffer.push(1));
    
    EXPECT_EQ(buffer.pop(), std::numeric_limits<int>::max());
    EXPECT_EQ(buffer.pop(), 1);
}

TEST_F(CircularBufferTest, ResetStateAfterClear) {
    EXPECT_TRUE(buffer.push(1));
    EXPECT_TRUE(buffer.push(2));
    
    EXPECT_EQ(buffer.pop(), 1);
    EXPECT_EQ(buffer.pop(), 2);
    
    // At this point, the buffer should be empty
    EXPECT_TRUE(buffer.is_empty());
    
    // Push new elements to verify the buffer state has been reset
    EXPECT_TRUE(buffer.push(3));
    EXPECT_TRUE(buffer.push(4));
    
    EXPECT_EQ(buffer.pop(), 3);
    EXPECT_EQ(buffer.pop(), 4);
}

// Move Semantics Test
TEST_F(CircularBufferTest, MoveSemantics) {
    struct MovableOnly {
        std::unique_ptr<int> data;

        // Default constructor
        MovableOnly() : data(nullptr) {}  // Sets data to nullptr, allowing default construction
        // Parameterized constructor
        explicit MovableOnly(int val) : data(std::make_unique<int>(val)) {}
        // Delete copy constructor and copy assignment to keep it move-only
        MovableOnly(const MovableOnly&) = delete;
        MovableOnly& operator=(const MovableOnly&) = delete;
        // Default move constructor and move assignment
        MovableOnly(MovableOnly&&) = default;
        MovableOnly& operator=(MovableOnly&&) = default;
    };
    
    spsc::CircularBuffer<MovableOnly, BUFFER_SIZE> move_buffer;
    
    // Test push with move-only type
    EXPECT_TRUE(move_buffer.push(MovableOnly(42)));
    
    // Test pop with move-only type
    auto item = move_buffer.pop();
    EXPECT_EQ(*item.data, 42);
}

// // Concurrent Operation Tests
// TEST_F(CircularBufferTest, ProducerConsumerScenario) {
//     static constexpr int NUM_ITEMS = 10000;
//     std::atomic<bool> producer_done{false};
//     std::vector<int> consumed_items;
//     consumed_items.reserve(NUM_ITEMS);
    
//     // Producer thread
//     std::thread producer([this, &producer_done]() {
//         for (int i = 0; i < NUM_ITEMS; ++i) {
//             while (!buffer.push(std::move(i))) {
//                 std::this_thread::yield();
//             }
//         }
//         producer_done = true;
//     });
    
//     // Consumer thread
//     std::thread consumer([this, &producer_done, &consumed_items]() {
//         while (!producer_done || !buffer.is_empty()) {
//             try {
//                 consumed_items.push_back(buffer.pop());
//             } catch (const std::runtime_error&) {
//                 std::this_thread::yield();
//             }
//         }
//     });
    
//     producer.join();
//     consumer.join();
    
//     // Verify all items were consumed in order
//     ASSERT_EQ(consumed_items.size(), NUM_ITEMS);
//     for (int i = 0; i < NUM_ITEMS; ++i) {
//         EXPECT_EQ(consumed_items[i], i);
//     }
// }

// // Stress Test
// TEST_F(CircularBufferTest, StressTest) {
//     static constexpr int NUM_CYCLES = 1000;
//     static constexpr int ITEMS_PER_CYCLE = BUFFER_SIZE * 2;
    
//     std::atomic<bool> should_continue{true};
//     std::atomic<int> producer_count{0};
//     std::atomic<int> consumer_count{0};
    
//     // Producer thread
//     std::thread producer([this, &should_continue, &producer_count]() {
//         while (should_continue) {
//             for (int i = 0; i < ITEMS_PER_CYCLE; ++i) {
//                 while (!buffer.push(std::move(i)) && should_continue) {
//                     std::this_thread::yield();
//                 }
//                 if (!should_continue) break;
//                 producer_count++;
//             }
//         }
//     });
    
//     // Consumer thread
//     std::thread consumer([this, &should_continue, &consumer_count]() {
//         while (should_continue) {
//             try {
//                 buffer.pop();
//                 consumer_count++;
//             } catch (const std::runtime_error&) {
//                 std::this_thread::yield();
//             }
//         }
//     });
    
//     // Run for a while
//     std::this_thread::sleep_for(std::chrono::seconds(2));
//     should_continue = false;
    
//     producer.join();
//     consumer.join();
    
//     // Verify counts match
//     EXPECT_EQ(producer_count.load(), consumer_count.load());
//     std::cout << "Processed " << producer_count << " items during stress test\n";
// }