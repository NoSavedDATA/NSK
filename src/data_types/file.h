#pragma once

#include <unordered_map>


constexpr int FileBufferSize = 8192;
constexpr int FileLineSize = 4096;

struct DT_file {
    int fd;
    char buffer[FileBufferSize];
    int line_counter, bytes_read, bytes_remaining;
};

void file_Clean_Up(void *);
