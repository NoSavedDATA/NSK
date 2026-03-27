

#include <map>
#include <string>
#include <vector>
#include "../nsk_cpp.h"



std::map<std::string, std::string> functions_return_type;

void set_functions_return_type() {

  functions_return_type = {{"gelu", "tensor"}, {"sigmoid", "tensor"}, {"_tanh", "tensor"}, {"relu", "tensor"}, {"softmax", "tensor"} , {"_glob_b_", "str_vec"}, {"glob", "str_vec"},
                           {"log", "tensor"}, {"randu_like", "tensor"}, {"RandomCrop", "tensor"}, {"RandomHorizontalFlip", "tensor"}, {"NormalizeImg", "tensor"},
                           {"dropout", "tensor"}, {"rl_discounted_return", "tensor"}, {"self_attn", "tensor"}, {"Jitter", "tensor"}, {"mse_with_priorities", "tensor"},
                           {"btc_mult", "tensor"}, {"btc_multT", "tensor"}, {"tensor_view", "tensor"}, {"clip", "tensor"}, {"tensor_argmax", "tensor"}, {"tmax", "tensor"},
                           {"tensor_onehot", "tensor"}, {"permute", "tensor"}, {"cpu", "tensor"}, {"printtt", "tensor"}, {"sum", "tensor"},
                           {"prod", "tensor"}, {"tensor_mean", "tensor"}, {"tmin", "tensor"}, {"argmin", "tensor"}, {"topk", "tensor"}, {"repeat_interleave", "tensor"},
                           {"save_img", "tensor"}, {"tensor_gpu", "tensor"}, {"save_as_int", "tensor"}, {"save_as_bin", "tensor"}, {"gather", "tensor"},
                           {"to_string", "str"}, {"cat_str_float", "str"}, {"Linear", "tensor"}, {"str_split_idx", "str"}, {"str_to_float", "float"},
                           {"mean_tensor", "tensor"},
                           {"BatchNorm2d", "tensor"}, {"Pool2d", "tensor"}, {"LSTM", "tensor"}, {"MHSA", "tensor"}, {"Embedding", "tensor"},
						{"IndexStrVec", "str"}, {"str_vec_Idx", "str"}, {"ShuffleStrVec", "str_vec"},
						{"print_vec_i8", "int"}, {"print_vec_i16", "int"}, {"print_vec_int", "int"}, {"print_vec_i64", "int"}, {"print_vec_float", "int"}, 
						{"print_randoms", "float"}, {"randint", "int"}, 
						{"object_Load_float", "float"}, {"object_Load_int", "int"}, {"object_Load_on_Offset_float", "float"}, {"object_Load_on_Offset_int", "int"}, 
						{"min", "float"}, {"max", "float"}, {"logE2f", "float"}, {"roundE", "float"}, {"floorE", "float"}, {"logical_not", "float"}, 
						{"LenStrVec", "int"}, {"ShuffleStrVec", "str_vec"}, {"shuffle_str", "str"}, {"IndexStrVec", "str"}, {"str_vec_Idx", "str"}, {"str_vec_CalculateIdx", "int"}, {"str_vec_print", "float"}, 
						{"bool_to_str", "str"}, {"bool_to_str_buffer", "int64_t"}, 
						{"print", "float"}, {"print_void_ptrC", "float"}, 
						{"list_New", "list"}, {"list_append", "float"}, {"list_print", "float"}, {"tuple_print", "float"}, {"list_Create", "list"}, {"list_shuffle", "float"}, {"list_size", "int"}, {"list_CalculateIdx", "int"}, {"to_int", "int"}, {"to_float", "float"}, {"to_bool", "bool"}, {"int_list_Store_Idx", "float"}, {"float_list_Store_Idx", "float"}, {"list_Store_Idx", "float"}, {"zip", "list"}, 
						{"_quit_", "float"}, {"fexists", "bool"}, 
						{"scope_struct_spec", "float"}, {"scope_struct_CreateFirst", ""}, {"scope_struct_Create", ""}, {"scope_struct_Overwrite", ""}, {"get_scope_thread_id", "int"}, {"scope_struct_Reset_Threads", "float"}, {"scope_struct_Increment_Thread", "float"}, 
						{"emerge_int", "int"}, {"emerge_float", "float"}, {"tid", "int"}, 
						{"str_Copy", "str"}, {"str_CopyArg", "str"}, {"str_str_add", "str"}, {"str_int_add", "str"}, {"str_float_add", "str"}, {"int_str_add", "str"}, {"float_str_add", "str"}, {"str_bool_add", "str"}, {"bool_str_add", "str"}, {"PrintStr", "float"}, {"cat_str_float", "str"}, {"str_split_idx", "str"}, {"can_convert_to_float", "bool"}, {"str_to_float", "float"}, {"str_str_different", "bool"}, {"str_str_equal", "bool"}, {"readline", "str"}, 
						{"channel_str_message", "float"}, {"channel_void_message", "float"}, {"str_channel_Idx", "str"}, {"str_channel_alive", "int"}, {"float_channel_message", "float"}, {"channel_float_message", "float"}, {"float_channel_Idx", "float"}, {"float_channel_sum", "float"}, {"float_channel_mean", "float"}, {"float_channel_terminate", "float"}, {"float_channel_alive", "int"}, {"int_channel_message", "int"}, {"channel_int_message", "float"}, {"int_channel_Idx", "int"}, {"int_channel_sum", "int"}, {"int_channel_mean", "float"}, {"int_channel_terminate", "float"}, {"int_channel_alive", "bool"}, 
						{"putchard", "float"}, {"printd", "float"}, 
						{"read_float", "float"}, {"float_to_str", "str"}, {"float_to_str_buffer", "int64_t"}, {"nsk_pow", "float"}, {"nsk_sqrt", "float"}, 
						{"is_null", "bool"}, 
						{"prebuild", "int"}, 
						{"dir_exists", "float"}, {"path_exists", "float"}, 
						{"__idx__", "int"}, {"__sliced_idx__", "int"}, 
						{"silent_sleep", "float"}, {"start_timer", "float"}, {"end_timer", "float"}, 
						{"read_int", "int"}, {"int_to_str", "str"}, {"i64_to_str_buffer", "int"}, {"int_to_str_buffer", "int"}, {"int_print_bits", "int"}, 
						{"charv_print", "int"}, 
						{"GetEmptyChar", "str"}, {"CopyString", "str"}, {"ConcatStr", "str"}, {"ConcatStrFreeLeft", "str"}, {"ConcatFloatToStr", "str"}, {"ConcatNumToStrFree", "str"}, 
						{"CreateNotesVector", "list"}, {"Dispose_NotesVector", "float"}, {"Add_To_NotesVector_float", "list"}, {"Add_To_NotesVector_int", "list"}, {"Add_To_NotesVector_str", "list"}, 
						{"array_Create", "array"}, {"array_size", "int"}, {"array_bad_idx", "int"}, {"arange_int", "array"}, {"zeros_int", "array"}, {"randint_array", "array"}, {"ones_int", "array"}, {"array_int_add", "array"}, {"randfloat_array", "array"}, {"array_print_float", "int"}, {"arange_float", "array"}, {"zeros_float", "array"}, {"ones_float", "array"}, {"array_Split_Parallel", "array"}, {"array_print_str", "int"}, 
						{"map_Create", "map"}, {"map_keys", "array"}, {"map_values", "array"}, 

	};

	functions_return_data_type["bool_to_str"] = Data_Tree("str");
	functions_return_data_type["bool_to_str_buffer"] = Data_Tree("int64_t");
	functions_return_data_type["_quit_"] = Data_Tree("float");
	functions_return_data_type["fexists"] = Data_Tree("bool");
	functions_return_data_type["prebuild"] = Data_Tree("int");
	functions_return_data_type["list_New"] = Data_Tree("list");
	functions_return_data_type["list_append"] = Data_Tree("float");
	functions_return_data_type["list_print"] = Data_Tree("float");
	functions_return_data_type["tuple_print"] = Data_Tree("float");
	functions_return_data_type["list_Create"] = Data_Tree("list");
	functions_return_data_type["list_shuffle"] = Data_Tree("float");
	functions_return_data_type["list_size"] = Data_Tree("int");
	functions_return_data_type["list_CalculateIdx"] = Data_Tree("int");
	functions_return_data_type["to_int"] = Data_Tree("int");
	functions_return_data_type["to_float"] = Data_Tree("float");
	functions_return_data_type["to_bool"] = Data_Tree("bool");
	functions_return_data_type["int_list_Store_Idx"] = Data_Tree("float");
	functions_return_data_type["float_list_Store_Idx"] = Data_Tree("float");
	functions_return_data_type["list_Store_Idx"] = Data_Tree("float");
	functions_return_data_type["zip"] = Data_Tree("list");
	functions_return_data_type["GetEmptyChar"] = Data_Tree("str");
	functions_return_data_type["CopyString"] = Data_Tree("str");
	functions_return_data_type["ConcatStr"] = Data_Tree("str");
	functions_return_data_type["ConcatStrFreeLeft"] = Data_Tree("str");
	functions_return_data_type["ConcatFloatToStr"] = Data_Tree("str");
	functions_return_data_type["ConcatNumToStrFree"] = Data_Tree("str");
	functions_return_data_type["scope_struct_spec"] = Data_Tree("float");
	functions_return_data_type["scope_struct_CreateFirst"] = Data_Tree("");
	functions_return_data_type["scope_struct_Create"] = Data_Tree("");
	functions_return_data_type["scope_struct_Overwrite"] = Data_Tree("");
	functions_return_data_type["get_scope_thread_id"] = Data_Tree("int");
	functions_return_data_type["scope_struct_Reset_Threads"] = Data_Tree("float");
	functions_return_data_type["scope_struct_Increment_Thread"] = Data_Tree("float");
	functions_return_data_type["print_vec_i8"] = Data_Tree("int");
	functions_return_data_type["print_vec_i16"] = Data_Tree("int");
	functions_return_data_type["print_vec_int"] = Data_Tree("int");
	functions_return_data_type["print_vec_i64"] = Data_Tree("int");
	functions_return_data_type["print_vec_float"] = Data_Tree("int");
	functions_return_data_type["silent_sleep"] = Data_Tree("float");
	functions_return_data_type["start_timer"] = Data_Tree("float");
	functions_return_data_type["end_timer"] = Data_Tree("float");
	functions_return_data_type["array_Create"] = Data_Tree("array");
	functions_return_data_type["array_size"] = Data_Tree("int");
	functions_return_data_type["array_bad_idx"] = Data_Tree("int");
	functions_return_data_type["arange_int"] = Data_Tree("array");
	functions_return_data_type["zeros_int"] = Data_Tree("array");
	functions_return_data_type["randint_array"] = Data_Tree("array");
	functions_return_data_type["ones_int"] = Data_Tree("array");
	functions_return_data_type["array_int_add"] = Data_Tree("array");
	functions_return_data_type["randfloat_array"] = Data_Tree("array");
	functions_return_data_type["array_print_float"] = Data_Tree("int");
	functions_return_data_type["arange_float"] = Data_Tree("array");
	functions_return_data_type["zeros_float"] = Data_Tree("array");
	functions_return_data_type["ones_float"] = Data_Tree("array");
	functions_return_data_type["array_Split_Parallel"] = Data_Tree("array");
	functions_return_data_type["array_print_str"] = Data_Tree("int");
	functions_return_data_type["str_Copy"] = Data_Tree("str");
	functions_return_data_type["str_CopyArg"] = Data_Tree("str");
	functions_return_data_type["str_str_add"] = Data_Tree("str");
	functions_return_data_type["str_int_add"] = Data_Tree("str");
	functions_return_data_type["str_float_add"] = Data_Tree("str");
	functions_return_data_type["int_str_add"] = Data_Tree("str");
	functions_return_data_type["float_str_add"] = Data_Tree("str");
	functions_return_data_type["str_bool_add"] = Data_Tree("str");
	functions_return_data_type["bool_str_add"] = Data_Tree("str");
	functions_return_data_type["PrintStr"] = Data_Tree("float");
	functions_return_data_type["cat_str_float"] = Data_Tree("str");
	functions_return_data_type["str_split_idx"] = Data_Tree("str");
	functions_return_data_type["can_convert_to_float"] = Data_Tree("bool");
	functions_return_data_type["str_to_float"] = Data_Tree("float");
	functions_return_data_type["str_str_different"] = Data_Tree("bool");
	functions_return_data_type["str_str_equal"] = Data_Tree("bool");
	functions_return_data_type["readline"] = Data_Tree("str");
	functions_return_data_type["read_float"] = Data_Tree("float");
	functions_return_data_type["float_to_str"] = Data_Tree("str");
	functions_return_data_type["float_to_str_buffer"] = Data_Tree("int64_t");
	functions_return_data_type["nsk_pow"] = Data_Tree("float");
	functions_return_data_type["nsk_sqrt"] = Data_Tree("float");
	functions_return_data_type["LenStrVec"] = Data_Tree("int");

	Data_Tree ShuffleStrVec_vec = Data_Tree("vec");
	ShuffleStrVec_vec.Nested_Data.push_back(Data_Tree("str"));
	functions_return_data_type["ShuffleStrVec"] = ShuffleStrVec_vec;
	functions_return_data_type["shuffle_str"] = Data_Tree("str");
	functions_return_data_type["IndexStrVec"] = Data_Tree("str");
	functions_return_data_type["str_vec_Idx"] = Data_Tree("str");
	functions_return_data_type["str_vec_CalculateIdx"] = Data_Tree("int");
	functions_return_data_type["str_vec_print"] = Data_Tree("float");
	functions_return_data_type["charv_print"] = Data_Tree("int");
	functions_return_data_type["emerge_int"] = Data_Tree("int");
	functions_return_data_type["emerge_float"] = Data_Tree("float");
	functions_return_data_type["tid"] = Data_Tree("int");
	functions_return_data_type["__idx__"] = Data_Tree("int");
	functions_return_data_type["__sliced_idx__"] = Data_Tree("int");
	functions_return_data_type["object_Load_float"] = Data_Tree("float");
	functions_return_data_type["object_Load_int"] = Data_Tree("int");
	functions_return_data_type["object_Load_on_Offset_float"] = Data_Tree("float");
	functions_return_data_type["object_Load_on_Offset_int"] = Data_Tree("int");
	functions_return_data_type["channel_str_message"] = Data_Tree("float");
	functions_return_data_type["channel_void_message"] = Data_Tree("float");
	functions_return_data_type["str_channel_Idx"] = Data_Tree("str");
	functions_return_data_type["str_channel_alive"] = Data_Tree("int");
	functions_return_data_type["float_channel_message"] = Data_Tree("float");
	functions_return_data_type["channel_float_message"] = Data_Tree("float");
	functions_return_data_type["float_channel_Idx"] = Data_Tree("float");
	functions_return_data_type["float_channel_sum"] = Data_Tree("float");
	functions_return_data_type["float_channel_mean"] = Data_Tree("float");
	functions_return_data_type["float_channel_terminate"] = Data_Tree("float");
	functions_return_data_type["float_channel_alive"] = Data_Tree("int");
	functions_return_data_type["int_channel_message"] = Data_Tree("int");
	functions_return_data_type["channel_int_message"] = Data_Tree("float");
	functions_return_data_type["int_channel_Idx"] = Data_Tree("int");
	functions_return_data_type["int_channel_sum"] = Data_Tree("int");
	functions_return_data_type["int_channel_mean"] = Data_Tree("float");
	functions_return_data_type["int_channel_terminate"] = Data_Tree("float");
	functions_return_data_type["int_channel_alive"] = Data_Tree("bool");
	functions_return_data_type["print_randoms"] = Data_Tree("float");
	functions_return_data_type["randint"] = Data_Tree("int");
	functions_return_data_type["dir_exists"] = Data_Tree("float");
	functions_return_data_type["path_exists"] = Data_Tree("float");
	functions_return_data_type["print"] = Data_Tree("float");
	functions_return_data_type["print_void_ptrC"] = Data_Tree("float");
	functions_return_data_type["is_null"] = Data_Tree("bool");
	functions_return_data_type["putchard"] = Data_Tree("float");
	functions_return_data_type["printd"] = Data_Tree("float");
	functions_return_data_type["read_int"] = Data_Tree("int");
	functions_return_data_type["int_to_str"] = Data_Tree("str");
	functions_return_data_type["i64_to_str_buffer"] = Data_Tree("int");
	functions_return_data_type["int_to_str_buffer"] = Data_Tree("int");
	functions_return_data_type["int_print_bits"] = Data_Tree("int");
	functions_return_data_type["CreateNotesVector"] = Data_Tree("list");
	functions_return_data_type["Dispose_NotesVector"] = Data_Tree("float");
	functions_return_data_type["Add_To_NotesVector_float"] = Data_Tree("list");
	functions_return_data_type["Add_To_NotesVector_int"] = Data_Tree("list");
	functions_return_data_type["Add_To_NotesVector_str"] = Data_Tree("list");
	functions_return_data_type["map_Create"] = Data_Tree("map");
	functions_return_data_type["map_keys"] = Data_Tree("array");
	functions_return_data_type["map_values"] = Data_Tree("array");
	functions_return_data_type["min"] = Data_Tree("float");
	functions_return_data_type["max"] = Data_Tree("float");
	functions_return_data_type["logE2f"] = Data_Tree("float");
	functions_return_data_type["roundE"] = Data_Tree("float");
	functions_return_data_type["floorE"] = Data_Tree("float");
	functions_return_data_type["logical_not"] = Data_Tree("float");

}