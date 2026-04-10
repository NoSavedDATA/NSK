#pragma once
#include <string>
#include "../mangler/scope_struct.h"

class DT_array {
    public:
		int virtual_size, size, elem_size;
		void *data=nullptr;
        uint16_t type;

    DT_array();
    void New(Scope_Struct*,int, int, int, uint16_t);
    void New(Scope_Struct*,int, int, uint16_t);
};


void array_Clean_Up(void *data_ptr, int);

extern "C" void array_double_size(Scope_Struct *scope_struct, DT_array *vec);
