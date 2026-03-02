#include <iostream>

#include "../mangler/scope_struct.h"


extern "C" int charv_print(Scope_Struct *scope_struct, char *c, int size) {
    std::cout << "print charv" << "\n";

    for (int i=0; i<size; ++i)
        std::cout << c[i];

    return 0;
}
