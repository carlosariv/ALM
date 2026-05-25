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

template<>
struct std::formatter<Atom*> : std::formatter<string_view> {
    auto format(const Atom *atom, std::format_context &ctx) const {
        std::string_view sv;
        if (atom && atom->len > 0) {
            sv = std::string_view((char *)atom->text, atom->len);
        } else {
            sv = "";
        }
        return std::formatter<std::string_view>::format(sv, ctx);
    }
};

extern Atomizer *atomizer;

void atomizer_init();
Atom *atom_create(String string);
Atom *atom_create(char *str, usize len);
String to_string(Atom *atom);
