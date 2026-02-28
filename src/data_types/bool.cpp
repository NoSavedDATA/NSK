#include <cstdlib>
#include <cstring>

#include "../mangler/scope_struct.h"


extern "C" char *bool_to_str(bool x) {
    const char *word = x ? "true" : "false";
    size_t len = std::strlen(word);

    char *result = (char *)std::malloc(len + 1);
    if (!result) return nullptr;

    std::memcpy(result, word, len + 1); // copy including '\0'
    return result;
}


extern "C" int64_t bool_to_str_buffer(Scope_Struct *scope_struct, bool x, char *buffer) {
    const char *word = x ? "true" : "false";
    size_t len = std::strlen(word);
    std::memcpy(buffer, word, len);
    return len;
}
