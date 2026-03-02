#include <cmath>
#include <iostream>
#include <limits>
#include <map>
#include <string>
#include <unordered_map>

#include "../../compiler_frontend/global_vars.h"
#include "../../compiler_frontend/logging_v.h"
#include "../../compiler_frontend/logging.h"
#include "../../clean_up/clean_up.h"
#include "../../data_types/list.h"
#include "../../mangler/scope_struct.h"
#include "../../pool/pool.h"
#include "../include.h"





int gc_sizes[GC_obj_sizes];
uint16_t GC_size_to_class[GC_N+1];
uint16_t GC_size_to_c[GC_N+1];

std::array<GC_span_traits*, GC_obj_sizes> GC_span_traits_vec;
std::unordered_map<int, std::vector<char *>> arena_base_addr;












