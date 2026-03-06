#pragma once

#include <functional>
#include <map>
#include <string>

extern std::map<std::string, std::function<void(void *, int)>> clean_up_functions;
