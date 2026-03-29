#include <cstddef>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../data_types/data_tree.h"

bool Shall_Exit = false;


// Tensor related
std::vector<std::string> return_tensor_functions, return_tensor_methods, return_tensor_fn, native_modules,
return_pinned_methods, vararg_methods, string_methods, native_methods, native_functions, native_fn,
return_string_fn, threaded_tensor_functions, require_scope_functions, notators_str, template_fn;

std::map<std::string, std::string> reverse_ops;

std::vector<std::string> user_cpp_functions;
std::map<std::string, int> Function_Arg_Count;
std::map<std::string, int> Function_Required_Arg_Count;
std::map<std::string, std::map<std::string, Data_Tree>> Function_Arg_DataTypes;
std::map<std::string, std::map<std::string, std::string>> Function_Arg_Types;
std::map<std::string, std::vector<std::string>> Function_Arg_Names;
std::vector<std::string> Sys_Arguments;


std::map<std::string, std::string> elements_type_return, ops_type_return;
std::map<int, std::string> op_map;
std::vector<std::string> op_map_names;
std::string CurrentFile = "main";

bool has_main=false;
bool IsJIT = true;

std::map<char, int> BinopPrecedence;



std::vector<std::string> Classes;
std::map<size_t, std::vector<char *>> CharPool;


std::map<std::string, std::vector<std::string>> Equivalent_Types = {{"int", {"float", "i64"}},
                                                                    {"i64", {"int"}},
                                                                    {"char", {"i8"}}};

std::vector<std::string> int_types = {"int", "i64", "i8", "i16", "char"};


std::unordered_map<std::string, uint16_t> data_name_to_size = {{"int", 4}, {"float", 4}, {"bool", 1}, {"any", 8}, {"double", 8}, {"str", 16}, {"str_view", 16}, {"float_ptr", 8}};

std::unordered_map<std::string, uint16_t> data_name_to_type = {{"int", 2}, {"float", 3}, {"bool", 4}, {"str", 5},
                                                               {"list", 6}, {"float_ptr", 23}, 
                                                               {"tuple", 7}, {"map", 8}, {"channel", 9}, {"int_vec", 10},
                                                               {"float_vec", 11}, {"array", 12}, {"map_node", 13},
                                                               {"char", 15},  {"charv", 16},
                                                               {"i64", 17}, {"i8", 18}, {"i16", 19}, {"vec", 20},
                                                               {"str_view", 21}, {"any", 22}};

std::unordered_map<uint16_t, std::string> data_type_to_name = {{2, "int"}, {3, "float"}, {4, "bool"}, {5, "str"},
                                                               {6, "list"}, {23, "float_ptr"},
                                                               {7, "tuple"}, {8, "map"}, {9, "channel"}, {10, "int_vec"},
                                                               {11, "float_vec"}, {12, "array"}, {13, "map_node"},
                                                               {15, "char"}, {16, "charv"},
                                                               {17, "i64"}, {18, "i8"}, {19, "i16"}, {20, "vec"},
                                                               {21, "str_view"}, {22, "any"}};
uint16_t data_type_count=24;



std::vector<std::string> data_tokens = {"tensor", "pinned_tensor", "int", "bool", "str", "str_vec", "float_vec",
                                        "MHSA", "LSTM", "Linear", "tuple",
                                        "any", "float_ptr",
										"list", "map", "array",
                                        "Embedding", "EmbeddingLn", "Conv2d", "Pool2d", "BatchNorm2d", "float", "int_vec", "char", "charv", "vec", "i16", "i64", "i8", "str_view"};
std::vector<std::string> compound_tokens = {"tuple", "list", "array", "map", "vec"};
std::vector<std::string> primary_data_tokens = {"vec", "int", "float", "bool", "foreach_control_var", "i64", "int8", "char"};


std::vector<uint16_t> primary_data_types = {2, 3, 4, 6, 15, 16, 17, 18, 21};
std::vector<uint16_t> compound_types = {6, 7, 8, 9, 12};
