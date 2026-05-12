#pragma once

#include <stdlib.h>

struct HeapAllocator {
    void *allocate(isize n) {
        return malloc(n);
    }

    void *allocate(isize n, const void *hint) {
        return realloc((void *)hint, n);
    }

    void deallocate(const void *ptr) {
        if (ptr) {
            free((void *)ptr);
        }
    }
};

template<int arena_size>
struct ArenaAllocator {
    struct Arena {
        void *data;
        isize offset;
        isize cap;
        Arena *prev;
    };

    Arena *arena;

    void *allocate(isize n) {
        return nullptr;
    }

    void deallocate() {
    }
};
