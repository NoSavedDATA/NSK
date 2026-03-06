#pragma once

#include <string>

#include "../mangler/scope_struct.h"

struct DT_str {
    char *str, *base;
    int len;
};

void str_Clean_Up(void *data, int);
