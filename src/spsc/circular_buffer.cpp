#pragma once
#include "../../include/spsc/circular_buffer.hpp"
#include<chrono>
#include<thread>


namespace spsc {

template<typename T, size_t Size>
T CircularBuffer<T, Size>::pop(unsigned max_attempts) {
    T item; // Declare the item to hold the popped value
    unsigned backoff = MIN_BACKOFF; // Initial backoff

    for (unsigned attempt = 0; attempt < max_attempts; ++attempt) {
        const size_t read_idx = read_index.value.load(std::memory_order_relaxed);
        const size_t write_idx = write_index.value.load(std::memory_order_acquire);
        bool is_full = full.load(std::memory_order_relaxed);

        // Check if the buffer is empty
        if (read_idx == write_idx && !is_full) {
            // Buffer is empty, apply backoff and retry
            backoff = std::min(backoff * 2, MAX_BACKOFF);  // Exponential backoff
            std::this_thread::sleep_for(std::chrono::microseconds(backoff));
            continue;
        }

        // Move the item from the buffer
        item = std::move(buffer[read_idx]);

        // Update read index for the next pop
        const size_t next_read = (read_idx + 1) & MASK;
        read_index.value.store(next_read, std::memory_order_release);

        // Update the full flag if the buffer is empty after popping
        full.store(false, std::memory_order_release);

        return item; 
    }

    throw std::runtime_error("Failed to pop item after multiple attempts"); // Consider throwing an exception instead of returning a default-constructed item
}


template<typename T, size_t Size>
bool CircularBuffer<T, Size>::push(T&& item, unsigned max_attempts) {
    unsigned backoff = MIN_BACKOFF;

    for (unsigned attempt = 0; attempt < max_attempts; ++attempt) {
        // Load current state of the buffer
        const size_t write_idx = write_index.value.load(std::memory_order_relaxed);

        if (full.load(std::memory_order_acquire)) {
            // Wait and retry if the buffer is full (exponential backoff)
            backoff = std::min(backoff * 2, MAX_BACKOFF);
            std::this_thread::sleep_for(std::chrono::microseconds(backoff));
            continue; // Retry the push operation
        }

        buffer[write_idx] = std::move(item);
        const size_t next_write_idx = (write_idx + 1) & MASK;
       
        write_index.value.store(next_write_idx, std::memory_order_release);

        // Check if buffer became full after pushing
        full.store(next_write_idx ==  read_index.value.load(std::memory_order_acquire), std::memory_order_release);
       
        return true; 
    } 
    return false; 
}

template<typename T, size_t Size>
T CircularBuffer<T, Size>::peek_oldest() const {
    // Check if the buffer is empty
    if (is_empty()) {
        throw std::runtime_error("Buffer is empty");
    }
    // Return the oldest item without modifying the buffer
    const size_t read_idx = read_index.value.load(std::memory_order_acquire);
    return buffer[read_idx]; 
}

template<typename T, size_t Size>

T  CircularBuffer<T, Size>::peek_latest() const {
    // Check if the buffer is empty
    if (is_empty()) {
        throw std::runtime_error("Circular buffer is empty");
    }
    // Return the latest item without modifying the buffer
    const size_t write_idx = write_index.value.load(std::memory_order_acquire);

    // The latest item is at (write_idx - 1) because write_idx points to the next free spot
    return buffer[(write_idx - 1) & MASK]; 
}

template<typename T, size_t Size>
bool CircularBuffer<T, Size>::is_empty() const {
        return !full.load(std::memory_order_acquire) && 
               (write_index.value.load(std::memory_order_acquire) == 
                read_index.value.load(std::memory_order_acquire));
}

template<typename T, size_t Size>
bool CircularBuffer<T, Size>::is_full() const {
    return full.load(std::memory_order_acquire);;
}

template<typename T, size_t Size>
size_t CircularBuffer<T, Size>::size() const {
    const size_t write_idx = write_index.value.load(std::memory_order_acquire);
    const size_t read_idx = read_index.value.load(std::memory_order_acquire);
    
    if (full.load(std::memory_order_acquire)) {
        return Size;
    }
        
    return (write_idx - read_idx) & MASK;
}

}