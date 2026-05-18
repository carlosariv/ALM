#pragma once

#include "BaseTypes.h"
#include "String.h"

struct Atom {
    Atom *next;
    u8 *text;
    int len;
};

struct AtomBucket {
    Atom *first = nullptr;
    Atom *last = nullptr;
    int count = 0;
};

struct AtomTable {
    AtomBucket *buckets = nullptr;
    int bucket_count = 0;
};

struct Atomizer {
    u32 hash_seed = 0;
    AtomTable *table = nullptr;
};

extern Atomizer *atomizer;

void atomizer_init();
Atom *atom_create(String string);
Atom *atom_create(char *str, usize len);
String get_string(Atom *atom);
