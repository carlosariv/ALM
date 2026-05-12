#pragma once

#include <initializer_list>
#include "BaseTypes.h"
#include "Allocator.h"

template <typename T, typename _Allocator = HeapAllocator>
struct Array {
    T *data = nullptr;
    isize count = 0;
    isize capacity = 0;
    _Allocator allocator;

    using iterator = T*;
    using const_iterator = const T*;

    Array() {
    }

    const T& operator[](isize i) const {
        return data[i];
    }
    T& operator[](isize i) {
        return data[i];
    }

    iterator begin() {
        return data;
    }
    const_iterator begin() const {
        return data;
    }
    iterator end() {
        return data + count;
    }
    const_iterator end() const {
        return data + count;
    }

    void __Grow(int amount) {
        isize new_count = count + amount;
        if (capacity < new_count) {
            int new_cap = capacity;
            while (new_cap < new_count) {
                new_cap = new_cap * 3 / 2 + 1;
            }
            data = static_cast<T*>(allocator.allocate(new_cap * sizeof(T), data));
            capacity = new_cap;
        }
    }

    void reserve(isize initial_size) {
        __Grow(initial_size);
    }

    void add(T elem) {
        __Grow(1);

        data[count] = elem;
        count += 1;
    }

    void add(std::initializer_list<T> elems) {
        __Grow(elems.size());
        for (T elem : elems) {
            data[count] = elem;
            count += 1;
        }
    }

    void remove() {
        if (count > 0) {
            count -= 1;
        }
    }

    void reset() {
        count = 0;
    }

    void release() {
        reset();
        allocator.deallocate(data);
        data = nullptr;
        capacity = 0;
    }
};
