#pragma once

#include <format>
#include "BaseTypes.h"

struct String {
    u8 *text = nullptr;
    isize len = 0;

    u8 const &operator[](isize i) const {
        return text[i];
    }

    bool operator==(const String &other) const {
        return len==other.len && (strncmp((char *)text, (char *)other.text, len)==0);
    }
};

struct StringHasher {
    size_t operator()(const String& k) const {
        size_t hash_seed = 5381;
        size_t hash = hash_seed;
        for (isize i = 0; i < k.len; i++) {
            hash = (hash << 5) + hash + k[i];
        }
        return hash;
        // return std::hash<isize>{}(k.len) ^ (std::hash<u8*>{}(k.text) << 1);
    }
};

template<>
struct std::formatter<String> {
    constexpr auto parse(std::format_parse_context &ctx) {
        return ctx.begin();
    }

    auto format(const String &s, std::format_context &ctx) const {
        return std::format_to(ctx.out(), "{:.{}}", (char *)s.text, s.len);
    }
};


String make_string(const char *text, isize len);
String make_string(u8 *text, isize len);
String string_concat(String const &a, String const &b);

#define str_lit(Name) make_string((Name), sizeof((Name))-1)
#define STRZ(Name) make_string((Name), sizeof((Name))-1)
