#pragma once
#include <string>
#include "../mangler/scope_struct.h"

struct DT_array_retire {
    DT_array_retire *next;
    void *data;
    int size, tid;
    DT_array_retire(void *data, int, int);
};

class DT_array {
    public:
		int virtual_size, size, elem_size;
		void *data=nullptr;
        DT_array_retire *retired=nullptr;
        uint16_t type;

    DT_array();
    void retire(int, int);
    void retire_clean();
    void New(Scope_Struct*,int, int, int, uint16_t);
    void New(Scope_Struct*,int, int, uint16_t);
};


void array_Clean_Up(void *data_ptr, int);

extern "C" void array_double_size(Scope_Struct *scope_struct, DT_array *vec);
