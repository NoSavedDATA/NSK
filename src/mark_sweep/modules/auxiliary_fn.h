#pragma once

#include <cmath>
#include <iostream>
#include <limits>
#include <map>
#include <string>
#include <unordered_map>

#include "../../mangler/scope_struct.h"


struct Scope_Struct;

std::string get_pool_obj_type(Scope_Struct *, void *);
