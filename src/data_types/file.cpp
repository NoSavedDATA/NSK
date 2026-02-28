

#include <filesystem>

#include "../mangler/scope_struct.h"
#include "file.h"


void file_Clean_Up(void *ptr) {
    // DT_file *file = static_cast<DT_file*>(ptr);
    // delete file->buffer;
}

extern "C" bool fexists(Scope_Struct *scope_struct, char *path) {
    return std::filesystem::exists(path);
}
