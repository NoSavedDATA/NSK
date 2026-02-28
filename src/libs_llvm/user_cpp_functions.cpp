

#include <map>
#include <string>
#include <vector>

#include "../include.h"
#include "../clean_up/clean_up.h"
#include "../compiler_frontend/include.h"

std::vector<std::string> user_cpp_functions;

void set_user_functions() {

    user_cpp_functions = {"Linear", "tensor_view", "tensor_clip", "tensor_argmax", "tensor_tmax", "tensor_onehot", "tensor_shape", "tensor_permute", "tensor_cpu", "printtt",
        "tensor_sum", "tensor_prod", "tensor_mean", "mean_tensor", "tensor_tmin", "tensor_argmin", "tensor_topk", "tensor_repeat_interleave",
        "tensor_save_img", "tensor_gpu", "tensor_gpuw", "tensor_save_as_int", "tensor_save_as_bin", "tensor_gather", "str_split_idx", "str_to_float", "list_print",
		"int_Create", "int_Load", "int_Store", 
		"fexists", 
		"read_int", "int_to_str", "int_to_str_buffer", 
		"dict_Create", "dict_New", "dict_Store_Key", "dict_Store_Key_int", "dict_Store_Key_float", "dict_print", "dict_Query", 
		"CreateNotesVector", "Dispose_NotesVector", "Add_To_NotesVector_float", "Add_To_NotesVector_int", "Add_To_NotesVector_str", 
		"list_New", "list_append_int", "list_append_float", "list_append_bool", "list_append", "list_print", "tuple_print", "list_Create", "list_shuffle", "list_size", "list_as_float_vec", "list_CalculateIdx", "to_int", "to_float", "to_bool", "list_CalculateSliceIdx", "list_Slice", "assign_wise_list_Idx", "int_list_Store_Idx", "float_list_Store_Idx", "list_Store_Idx", "zip", "list_Idx", "tuple_Idx", 
		"_quit_", 
		"array_Create", "array_size", "array_bad_idx", "array_double_size", "array_print_int", "arange_int", "zeros_int", "randint_array", "ones_int", "array_int_add", "randfloat_array", "array_print_float", "arange_float", "zeros_float", "ones_float", "array_print_str", "array_Split_Parallel", 
		"print_stack1", "print_stack", "scope_struct_spec", "set_scope_line", "scope_struct_CreateFirst", "scope_struct_Create", "scope_struct_Overwrite", "set_scope_thread_id", "get_scope_thread_id", "scope_struct_Reset_Threads", "scope_struct_Increment_Thread", "scope_struct_Print", "scope_struct_Save_for_Async", "scope_struct_Load_for_Async", "scope_struct_Store_Asyncs_Count", "scope_struct_Get_Async_Scope", "scope_struct_Clear_GC_Root", "scope_struct_Add_GC_Root", "scope_struct_Sweep", "scope_struct_Delete", 
		"Delete_Ptr", 
		"min", "max", "logE2f", "roundE", "floorE", "logical_not", 
		"channel_Create", "str_channel_message", "channel_str_message", "void_channel_message", "channel_void_message", "str_channel_Idx", "str_channel_terminate", "str_channel_alive", "float_channel_message", "channel_float_message", "float_channel_Idx", "float_channel_sum", "float_channel_mean", "float_channel_terminate", "float_channel_alive", "int_channel_message", "channel_int_message", "int_channel_Idx", "int_channel_sum", "int_channel_mean", "int_channel_terminate", "int_channel_alive", 
		"LockMutex", "UnlockMutex", 
		"LogErrorCall", "print_codegen", "print_codegen_silent", 
		"bool_to_str", "bool_to_str_buffer", 
		"offset_object_ptr", "object_Attr_float", "object_Attr_int", "object_Load_float", "object_Load_int", "object_Load_slot", "tie_object_to_object", "object_Attr_on_Offset_float", "object_Attr_on_Offset_int", "object_Attr_on_Offset", "object_Load_on_Offset_float", "object_Load_on_Offset_int", "object_Load_on_Offset", "object_ptr_Load_on_Offset", "object_ptr_Attribute_object", 
		"print_randoms", "randint", 
		"get_barrier", 
		"__slee_p_", "random_sleep", "silent_sleep", "start_timer", "end_timer", 
		"scope_struct_Alloc_GC", 
		"str_Create", "str_Copy", "str_CopyArg", "str_str_add", "str_int_add", "str_float_add", "int_str_add", "float_str_add", "str_bool_add", "bool_str_add", "PrintStr", "cat_str_float", "str_split_idx", "str_to_float", "str_str_different", "str_str_equal", "str_Delete", "readline", 
		"dive_void", "dive_int", "dive_float", "emerge_void", "emerge_int", "emerge_float", "tid", "pthread_create_aux", "pthread_join_aux", "pthread_create_aux", "pthread_join_aux", 
		"print", "print_void_ptr", "print_int", "print_int64", "print_uint64", 
		"dir_exists", "path_exists", 
		"allocate_void", 
		"str_vec_Create", "LenStrVec", "ShuffleStrVec", "shuffle_str", "IndexStrVec", "str_vec_Idx", "str_vec_CalculateIdx", "str_vec_print", 
		"map_Create", "map_expand", "print_str", "map_print", "map_keys", "map_values", "map_bad_key_str", "map_bad_key_int", "map_bad_key_float", 
		"read_float", "float_to_str", "float_to_str_buffer", "nsk_pow", "nsk_sqrt", 
		"nullptr_get", "is_null", 
		"float_vec_Create", "float_vec_first_nonzero", "float_vec_print", "float_vec_pow", "float_vec_sum", "float_vec_int_add", "float_vec_int_div", "float_vec_float_vec_add", "float_vec_float_vec_sub", "float_vec_Split_Parallel", "float_vec_Split_Strided_Parallel", "float_vec_size", 
		"int_vec_Create", "nsk_vec_size", "int_vec_CalculateSliceIdx", "int_vec_Slice", "int_vec_print", "int_vec_Split_Parallel", "int_vec_Split_Strided_Parallel", "int_vec_size", 
		"__idx__", "__sliced_idx__", 
		"GetEmptyChar", "FreeCharFromFunc", "FreeChar", "CopyString", "ConcatStr", "ConcatStrFreeLeft", "ConcatFloatToStr", "ConcatNumToStrFree", 

	};


	clean_up_functions["int_vec"] = int_vec_Clean_Up;

	clean_up_functions["str"] = str_Clean_Up;

	clean_up_functions["array"] = array_Clean_Up;

	clean_up_functions["float_vec"] = float_vec_Clean_Up;

	clean_up_functions["str_vec"] = str_vec_Clean_Up;

	clean_up_functions["list"] = list_Clean_Up;

	clean_up_functions["channel"] = channel_Clean_Up;

	clean_up_functions["file"] = file_Clean_Up;

	clean_up_functions["map_node"] = map_node_Clean_Up;

	clean_up_functions["map"] = map_Clean_Up;

	clean_up_functions["dict"] = dict_Clean_Up;


}