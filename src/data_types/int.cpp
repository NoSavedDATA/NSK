#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include <string>

#include "../codegen/string.h"
#include "../common/extension_functions.h"
#include "../mangler/scope_struct.h"
#include "../pool/include.h"
#include "include.h"



extern "C" int read_int(Scope_Struct *scope_struct) {
    int value;
    if (scanf("%d", &value) != 1) {
        fprintf(stderr, "Failed to read int\n");
        return 0; // default on failure
    }
    return value;
}


// extern "C" char *int_to_str(Scope_Struct *scope_struct, int x) {
//     // Enough for 32-bit int including sign and null terminator
//     char buffer[32];  
//     int len = std::snprintf(buffer, sizeof(buffer), "%d", x);

//     if (len < 0) return nullptr;  // snprintf error

//     char *result = allocate<char>(scope_struct, len+1, "str");
//     if (!result) return nullptr;  // malloc failed

//     std::memcpy(result, buffer, len + 1); // copy with '\0'

//     return result;
// }


extern "C" char* int_to_str(Scope_Struct *scope_struct, int value)
{
    // Handle special case: minimum int
    if (value == std::numeric_limits<int>::min()) {
        const char* min_str = "-2147483648";
        char *result = allocate<char>(scope_struct, 12, "str");
        std::memcpy(result, min_str, 12);
        return result;
    }

    bool negative = (value < 0);
    if (negative)
        value = -value;

    // Count digits
    int temp = value;
    int digits = 1;
    while (temp >= 10) {
        temp /= 10;
        ++digits;
    }

    int total_len = digits + (negative ? 1 : 0);

    char *result = allocate<char>(scope_struct, total_len+1, "str");
    result[total_len] = '\0';

    // Fill digits from end
    for (int i = 0; i < digits; ++i) {
        result[total_len - 1 - i] = '0' + (value % 10);
        value /= 10;
    }

    if (negative)
        result[0] = '-';

    return result;
}


extern "C" int64_t int_to_str_buffer(Scope_Struct *scope_struct, int value, char *buffer)
{
    // Handle special case: minimum int
    if (value == std::numeric_limits<int>::min()) {
        const char* min_str = "-2147483648";
        std::memcpy(buffer, min_str, 12);
        return 12;
    }

    bool negative = (value < 0);
    if (negative)
        value = -value;

    // Count digits
    int temp = value;
    int digits = 1;
    while (temp >= 10) {
        temp /= 10;
        ++digits;
    }

    int total_len = digits + (negative ? 1 : 0);

    // Fill digits from end
    for (int i = 0; i < digits; ++i) {
        buffer[total_len - 1 - i] = '0' + (value % 10);
        value /= 10;
    }

    if (negative)
        buffer[0] = '-';

    return total_len;
}
