#include <stdlib.h>
#include <cstring>
#include "String.h"

String make_string(const char *text, isize len) {
    return make_string((u8 *)text, len);
}

String make_string(u8 *text, isize len) {
    String string;
    string.len = len;
    string.text = (u8 *)malloc((len + 1) * sizeof(u8));
    if (len > 0) {
        std::memcpy(string.text, text, len);
    }
    string.text[len] = 0;
    return string;
}

String string_concat(String const &a, String const &b) {
    isize len = a.len + b.len;
    u8 *text = new u8[len + 1];
    std::memcpy(text, a.text, a.len);
    std::memcpy(text + a.len, b.text, b.len);
    text[len] = 0;
    return make_string(text, len);
}
