#pragma once


#include "../mangler/scope_struct.h"

#define SENTINEL_BYTE 0xCD

bool check_initialized_field(void *);


struct Scope_Struct;
extern "C" void *allocate_pool(Scope_Struct *scope_struct, int size, uint16_t type_id);
