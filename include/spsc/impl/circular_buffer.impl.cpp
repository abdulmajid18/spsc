#pragma once
#include "../circular_buffer.cpp"

namespace spsc {

template<typename T, size_t Size>
CircularBuffer<T, Size>::CircularBuffer() : head(0), tail(0), full(false) {}

template<typename T, size_t Size>
T CircularBuffer<T, Size>::pop(unsigned max_attempts = 3) {
    unsigned backoff = MIN_BACKOFF; // Initial small backoff

    for (unsigned attempt = 0; attempt < max_attempts; ++attempt) {
        size_t current_head = head.load(std::memory_order_acquire);
        size_t current_tail = tail.load(std::memory_order_relaxed);
        bool is_full = full.load(std::memory_order_relaxed);

        if (!is_full && current_head == current_tail) {
            // Buffer is empty, apply backoff and retry
            backoff = std::min(backoff * 2, MAX_BACKOFF);  // Exponential backoff
            std::this_thread::sleep_for(std::chrono::microseconds(backoff));
            continue;
        }

        item = std::move(buffer[current_tail]);

        size_t new_tail = (current_tail + 1) & MASK;
        tail.store(new_tail, std::memory_order_release);

        full.store(false, std::memory_order_release);

        return true; 
    }

    return false;  
}


template<typename T, size_t Size>
bool CircularBuffer<T, Size>::push(T item, unsigned max_attempts = 3) {
    unsigned backoff = MIN_BACKOFF;

    for (unsigned attempt = 0; attempt < max_attempts; ++attempt) {
        // Load current state of the buffer
        size_t current_head = head.load(std::memory_order_relaxed);
        size_t current_tail = tail.load(std::memory_order_acquire);
        bool is_full = full.load(std::memory_order_relaxed);

        if (is_full) {
            // Wait and retry if the buffer is full (exponential backoff)
            backoff = std::min(backoff * 2, MAX_BACKOFF);
            std::this_thread::sleep_for(std::chrono::microseconds(backoff));
            continue; // Retry the push operation
        }

        size_t new_head = (current_head + 1) & MASK;
        buffer[current_head] = std::move(item);
        head.store(new_head, std::memory_order_release);

        // Check if buffer became full after pushing
        if (new_head == current_tail) {
            full.store(true, std::memory_order_release);
        }
        return true; 
    } 
    return false; 
}

template<typename T, size_t Size>
T CircularBuffer<T, Size>::peek_oldest() const {
    if (is_empty()) {
        throw std::runtime_error("Buffer is empty");
    }
    return buffer[tail]; 
}

template<typename T, size_t Size>

T  CircularBuffer<T, Size>::peek_latest() const {
    if (is_empty()) {
        throw std::runtime_error("Circular buffer is empty");
    }
    return buffer[(head - 1) & MASK]; 
}

template<typename T, size_t Size>
bool CircularBuffer<T, Size>::is_empty() const {
    return !full.load(std::memory_order_relaxed) && 
               (head.load(std::memory_order_relaxed) == 
                tail.load(std::memory_order_relaxed));
}

template<typename T, size_t Size>
bool CircularBuffer<T, Size>::is_full() const {
    return full.load(std::memory_order_acquire);;
}

template<typename T, size_t Size>
size_t CircularBuffer<T, Size>::size() const {
    size_t current_head = head.load(std::memory_order_relaxed);
    size_t current_tail = tail.load(std::memory_order_relaxed);
    bool full = full.load(std::memory_order_relaxed);

    if (full) return Size;
    
    return (current_head - current_tail) & MASK;
}

}