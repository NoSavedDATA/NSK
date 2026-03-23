#include <cstdlib>
#include <filesystem>

#include "../mangler/scope_struct.h"
namespace fs = std::filesystem;

extern "C" float _quit_(Scope_Struct *scope_struct) {
    std::exit(0);
    return 0;
}


extern "C" bool fexists(Scope_Struct *scope_struct, char *file) {
    return fs::exists(file);
}
