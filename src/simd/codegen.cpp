#include "../nsk_cpp_llvm.h"
// std::unordered_map<std::string, std::function<Data_Tree(Parser_Struct, std::vector<std::unique_ptr<ExprAST>>&)>>function_return_overwrite;


Data_Tree simd_load_ret(Parser_Struct parser_struct, std::vector<std::unique_ptr<ExprAST>>& Args) {
  Data_Tree dt = Data_Tree("vec");
  auto stmt_1 = dynamic_cast<IntExprAST*>(Args[1].get());
  auto stmt_2 = dynamic_cast<IntExprAST*>(Args[2].get());
  dt.Nested_Data.push_back(Data_Tree(data_type_to_name[stmt_1->Val]));
  dt.Nested_Data.push_back(Data_Tree(std::to_string(stmt_2->Val)));
  return dt;
}

// __m256i chunk = _mm256_loadu_si256((__m256i*)ptr);
Value *simd_load(Parser_Struct parser_struct, Function *TheFunction,
                 std::string Callee, Data_Tree data_type, std::vector<Data_Tree> &args_type,
                 Value *scope_struct, std::vector<std::unique_ptr<ExprAST>> &Args, std::vector<Value*> &ArgsV) {
    llvm::Type *ty = get_type_from_data(data_type);

    Value *vecptr = Builder->CreateBitCast(ArgsV[0], Builder->getPtrTy());
    auto *L = Builder->CreateLoad(ty, ArgsV[0]);
    L->setAlignment(llvm::Align(1));
    return L;
}
