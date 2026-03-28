#include "../../compiler_frontend/tokenizer.h"
#include "../../libs_llvm/functions_args.h"
#include "../../libs_llvm/functions_return.h"
#include "../../libs_llvm/user_cpp_functions.h"
#include "../compiler_frontend/global_vars.h"
#include "../include.h"



// void prebuild() {
extern "C" int prebuild() {

    has_main=true;

  // Defines standard binary operators.
  // 1 is lowest precedence.
  BinopPrecedence[tok_space] = 1;
  BinopPrecedence['$'] = 1;
  BinopPrecedence['='] = 4;
  BinopPrecedence[tok_arrow] = 4;
  BinopPrecedence['!'] = 9;
  BinopPrecedence[tok_and] = 9;
  BinopPrecedence[tok_not] = 9;
  BinopPrecedence[tok_or] = 9;
  BinopPrecedence[tok_xor] = 9;
  BinopPrecedence['>'] = 10;
  BinopPrecedence['<'] = 10;
  BinopPrecedence[tok_equal] = 10;
  BinopPrecedence[tok_diff] = 10;
  BinopPrecedence[tok_minor_eq] = 10;
  BinopPrecedence[tok_higher_eq] = 10;
  BinopPrecedence[tok_offby] = 19;
  BinopPrecedence['+'] = 20;
  BinopPrecedence['-'] = 20;
  BinopPrecedence['%'] = 35;
  BinopPrecedence['*'] = 39;
  BinopPrecedence['/'] = 40;
  BinopPrecedence[tok_int_div] = 40;
  BinopPrecedence['^'] = 50;
  BinopPrecedence['@'] = 60;



    gc_sizes[0] = 8;
    gc_sizes[1] = 16;
    gc_sizes[2] = 24;
    gc_sizes[3] = 48;
    gc_sizes[4] = 64;
    gc_sizes[5] = 128;
    gc_sizes[6] = 256;
    gc_sizes[7] = 384;
    gc_sizes[8] = 512;
    gc_sizes[9] = 768;
    gc_sizes[10] = 1024;
    gc_sizes[11] = 2048;
    gc_sizes[12] = 4096;
    gc_sizes[13] = 8192;
    gc_sizes[14] = GC_max_object_size;


    for (int i=0, c=0; i<=GC_N; i++) {
        int size = i * GC_ALIGN;
        while (c<GC_obj_sizes-1 && gc_sizes[c]<size)
            c++;
        GC_size_to_class[i] = gc_sizes[c];
        GC_size_to_c[i] = c;
    }
    for (int i=0; i<GC_obj_sizes; ++i)
        GC_span_traits_vec[i] = new GC_span_traits(gc_sizes[i]);

    for (int i=0; i<4096; ++i)
        type_info[i] = nullptr;
        



  Data_Tree aux = Data_Tree("array");
  aux.Nested_Data.push_back(Data_Tree("int"));
  functions_return_data_type["randint_array"] = aux;
  functions_return_data_type["arange_int"] = aux;
  functions_return_data_type["array_int_add"] = aux;
  functions_return_data_type["zeros_int"] = aux;
  functions_return_data_type["ones_int"] = aux;
  aux.Nested_Data[0] = Data_Tree("float");
  functions_return_data_type["randfloat_array"] = aux;
  functions_return_data_type["arange_float"] = aux;
  functions_return_data_type["array_float_add"] = aux;
  functions_return_data_type["zeros_float"] = aux;
  functions_return_data_type["ones_float"] = aux;

  Data_Tree split_str_dt = Data_Tree("array");
  split_str_dt.Nested_Data.push_back(Data_Tree("str")); 
  functions_return_data_type["str_split"] = split_str_dt;




  
  
  functions_return_data_type["scope_struct_Sweep"] = Data_Tree("float");



  
  vararg_methods = {"zip"};
  template_fn = {"simd_load"};

  // tensor + string + ...
  // e.g: x.view(), str.split()
  native_methods = {"err", "split", "split_idx", "str_vec_print", "file_open", "file_read", "file_close", "file_open",
                    "file_opened", "c_open", "c_read", "c_strlen", "c_memchr", "c_memcpy", "fexists", "str_set", "vec_print", "vec_movemask", "int_print_bits"};
  native_methods = concat_str_vec(native_methods, user_cpp_functions);

  return_string_fn = {"to_string", "cat_str_float"};


  native_functions = {"ShuffleStrVec", "gload_img", "wload_img", "silent_sleep", "__slee_p_",
                      "LenStrVec", "zeros_vec", "ones_vec", "start_timer", "end_timer",
                      "_glob_b_", "print", "cross_entropy", "backprop", "AdamW", "SGD",
                      "load_preprocess_img", "max", "min", "unbug",
                      "cpu_idx", "OneCycleLR", "CosineLR", "wload_img_resize",
                      "build_vocab", "tokenize", "wtokenize", "write_zerosw",
                      "wtokenize_pad_left", "print_randoms", "wtokenize_pad_left_batch_first",
                      "wtokenize_pad_left_idx", "print_scope", "load_bin", "wload_bin", "randint",
                      "print_tensor", "path_exists", "dir_exists", "load_bin_idx",
                      "network_ema", "mse", "priority_sample", "priority_sample_val",
                      "importance_sample_idx", "importance_sample_weight",
                      "cross_entropy_idx"};
  native_functions = concat_str_vec(native_functions, return_string_fn);
  native_fn = concat_str_vec(native_methods, native_functions);



  reverse_ops = {{"float_tensor", "tensor_float"}};

  elements_type_return = {{"tensor_tensor", "tensor"}, {"float_float", "float"}, {"str_str", "str"}, {"str_float", "str"},
                     {"float_str", "str"}, {"int_int", "int"}, {"int_float", "float"}, {"float_int", "float"}, {"str_int", "str"}, {"int_str", "str"},
                     {"str_bool", "str"}, {"bool_str", "str"}, {"bool_bool", "bool"},
                     {"tensor_float", "tensor"}, {"pinned_tensor_pinned_tensor", "pinned_tensor"},
                     {"pinned_tensor_tensor", "pinned_tensor"}, {"pinned_tensor_float", "pinned_tensor"},
                     {"object_object", "object"}, {"str_object", "object"},
                     {"tensor_int", "tensor"}, {"int_tensor", "tensor"}, {"str_channel", "str"},
                     {"channel_str", "float"}, {"channel_int", "float"},
                     {"int_channel", "int"}, {"channel_float", "float"}, {"float_channel", "float"}, {"i64_i64", "i64"}};

  ops_type_return = {{"int_int_higher", "bool"}, {"int_int_minor", "bool"},
                     {"int_int_equal", "bool"},  {"int_int_different", "bool"},
                     {"int_int_higher_eq", "bool"}, {"int_int_minor_eq", "bool"},
                     {"i8_i8_higher", "bool"}, {"i8_i8_minor", "bool"},
                     {"i8_i8_equal", "bool"},  {"i8_i8_different", "bool"},
                     {"i8_i8_higher_eq", "bool"}, {"i8_i8_minor_eq", "bool"},
                     {"i16_higher", "bool"}, {"i16_minor", "bool"},
                     {"i16_equal", "bool"},  {"i16_different", "bool"},
                     {"i16_higher_eq", "bool"}, {"i16_minor_eq", "bool"},
                     {"i64_i64_higher", "bool"}, {"i64_i64_minor", "bool"},
                     {"i64_i64_equal", "bool"},  {"i64_i64_different", "bool"},
                     {"i64_i64_higher_eq", "bool"}, {"i64_i64_minor_eq", "bool"},
                     {"float_float_higher", "bool"}, {"float_float_minor", "bool"},
                     {"float_float_equal", "bool"}, {"float_float_different", "bool"},
                     {"float_float_higher_eq", "bool"}, {"float_float_minor_eq", "bool"},
                     {"str_int_offby", "str"},
                     {"charv_int_add", "charv"}, {"charv_i64_add", "charv"}};

                     

  op_map = {{'*', "mult"}, {'@', "mma"},  {'+', "add"}, {'-', "sub"}, {'/', "div"}, {'<', "minor"},
            {'>', "higher"}, {tok_equal, "equal"},
            {tok_diff, "different"}, {tok_higher_eq, "higher_eq"}, {tok_minor_eq, "minor_eq"}, {'%', "mod"}, {'=', "attr"},
            {77, "error"}, {tok_arrow, "message"}, {tok_and, "and"}, {tok_not, "not"}, {tok_or, "or"},
            {tok_xor, "xor"}, {tok_offby, "offby"}};

  for (auto pair : op_map)
    op_map_names.push_back(pair.second);


  
  notators_str = {"bias", "fp32", "fp16", "causal"};

  return 0;
}
