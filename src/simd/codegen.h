#pragma once
#include "../nsk_cpp.h"

Data_Tree simd_load_ret(Parser_Struct parser_struct, std::vector<std::unique_ptr<ExprAST>>& Args);

Value *simd_load(Parser_Struct parser_struct, Function *TheFunction,
                 std::string Callee, Data_Tree data_type, std::vector<Data_Tree> &args_type,
                 Value *scope_struct, std::vector<std::unique_ptr<ExprAST>> &Args, std::vector<Value*> &ArgsV);

