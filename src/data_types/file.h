#pragma once

#include <unordered_map>

struct DT_file {
    int fd;
    char buffer[1024];
    int line_counter, bytes_read, bytes_remaining;
    char *line;
};
