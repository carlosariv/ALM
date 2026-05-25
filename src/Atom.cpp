#include <cstring>
#include <unordered_set>

#include "String.h"
#include "Atom.h"

Atomizer *atomizer;

void atomizer_init() {
    atomizer = new Atomizer();
    atomizer->hash_seed = 5381;
    atomizer->table = new AtomTable;
    atomizer->table->bucket_count = 128;
    atomizer->table->buckets = new AtomBucket[atomizer->table->bucket_count];
    memset(atomizer->table->buckets, 0, 128 * sizeof(AtomBucket));
}

u32 string_hash(String string) {
    u32 hash_seed = 5381;
    u32 hash = hash_seed;
    for (int i = 0; i < string.len; i++) {
        hash = ((hash << 5) + hash) + (u32)string.text[i];
    }
    return hash;
}

Atom *atom_search(String string) {
    u32 hash = string_hash(string);
    u32 bucket_index = hash % atomizer->table->bucket_count;
    AtomBucket *bucket = atomizer->table->buckets + bucket_index;

    for (Atom *atom = bucket->first; atom; atom = atom->next) {
        if (atom->len == string.len && strncmp((char *)atom->text, (char *)string.text, string.len) == 0) {
            return atom;
        }
    }
    return nullptr;
}

Atom *atom_create(String string) {
    Atom *found = atom_search(string);
    if (found) {
        return found;
    }

    u32 hash = string_hash(string);
    u32 bucket_index = hash % atomizer->table->bucket_count;
    AtomBucket *bucket = atomizer->table->buckets + bucket_index;

    Atom *atom = new Atom;
    atom->next = nullptr;
    atom->text = string.text;
    atom->len = string.len;

    if (bucket->last != nullptr) {
        bucket->last->next = atom;
    } else {
        bucket->first = atom;
    }
    bucket->last = atom;

    return atom;
}

Atom *atom_create(char *str, usize len) {
    return atom_create(String((u8 *)str, len));
}

String to_string(Atom *atom) {
    return String(atom->text, atom->len);
}
