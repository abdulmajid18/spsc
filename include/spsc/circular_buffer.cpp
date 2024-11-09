#pragma once

#include<iostream>
#include<array>
#include<atomic>

namespace spsc {

template<typename T, size_t Size>
class CircularBuffer {
    static_assert(Size > 0, "Buffer size must be greater than zero");
    static_assert(std::is_default_constructible_v<T>, "T must be default constructible");
    static_assert((Size & (Size - 1)) == 0, "Size must be a power of 2");

    public:
        explicit CircularBuffer() : capacity(capacity), head(0), tail(0), full(false) {}

        bool push(T item, unsigned max_attempts = 3);
        T pop(unsigned max_attempts = 3);
        T peek_oldest() const;
        T peek_latest() const;
        bool is_empty() const;
        bool is_full() const;
        size_t size() const;
        static constexpr size_t capacity() {
                return Size;
        }

    private:
        std::array<T, Size> buffer;
        std::atomic<size_t> head;
        std::atomic<size_t> tail;
        std::atomic<size_t> capacity;
        std::atomic<bool full;
        static constexpr size_t MASK = Size - 1;
        static constexpr unsigned MAX_BACKOFF = 32;
        static constexpr unsigned MIN_BACKOFF = 1;

};

}

// #include "./impl/circular_buffer.impl.cpp"