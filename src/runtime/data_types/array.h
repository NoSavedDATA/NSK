#pragma once
#include <string>
#include "../mangler/scope_struct.h"

class DT_array {
    public:
		int virtual_size, size, elem_size;
		void *data=nullptr;
        std::string type;

    DT_array();
    void New(Scope_Struct*,int, int, int, std::string);
    void New(Scope_Struct*,int, int, std::string);
};


void array_Clean_Up(void *data_ptr, int);

