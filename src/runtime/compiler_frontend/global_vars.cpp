#include <cstddef>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>


bool Shall_Exit = false;


// Tensor related
std::vector<std::string> return_tensor_functions, return_tensor_methods, return_tensor_fn, native_modules,
return_pinned_methods, vararg_methods, string_methods, native_methods, native_functions, native_fn,
return_string_fn, threaded_tensor_functions, require_scope_functions, notators_str, template_fn;

std::map<std::string, std::string> reverse_ops;

std::vector<std::string> Sys_Arguments;


std::string CurrentFile = "main";

std::map<std::string, std::string> floatFunctions;
bool has_main=false;




std::vector<std::string> Classes;
std::map<size_t, std::vector<char *>> CharPool;


std::map<std::string, std::vector<std::string>> Equivalent_Types = {{"int", {"float", "i64"}},
                                                                    {"i64", {"int"}},
                                                                    {"char", {"i8"}}};




std::vector<std::string> data_tokens = {"tensor", "pinned_tensor", "int", "bool", "str", "str_vec", "float_vec", "MHSA", "LSTM", "Linear", "tuple",
										"list", "map", "array",
                                        "Embedding", "EmbeddingLn", "Conv2d", "Pool2d", "BatchNorm2d", "float", "int_vec", "char", "charv", "vec", "i16", "i64", "i8", "str_view"};
std::vector<std::string> compound_tokens = {"tuple", "list", "array", "map", "vec"};
std::vector<std::string> primary_data_tokens = {"vec", "int", "float", "bool", "foreach_control_var", "i64", "int8", "char"};


std::vector<uint16_t> primary_data_types = {2, 3, 4, 6, 15, 16, 17, 18, 21};
std::vector<uint16_t> compound_types = {6, 7, 8, 9, 12};
