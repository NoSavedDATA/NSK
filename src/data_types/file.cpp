

#include <filesystem>

#include "../mangler/scope_struct.h"
#include "file.h"


void file_Clean_Up(void *ptr, int tid) {
}

extern "C" bool fexists(Scope_Struct *scope_struct, char *path) {
    return std::filesystem::exists(path);
}
