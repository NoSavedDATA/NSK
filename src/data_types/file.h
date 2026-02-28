#pragma once

#include <unordered_map>


constexpr int FileBufferSize = 8192;
constexpr int FileLineSize = 1024;

struct DT_file {
    int fd;
    char buffer[1024];
    int line_counter, bytes_read, bytes_remaining;
};

void file_Clean_Up(void *);
