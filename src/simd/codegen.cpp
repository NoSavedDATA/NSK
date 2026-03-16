#include "../nsk_cpp_llvm.h"


Value *ctz(Parser_Struct parser_struct, Function *TheFunction,
                 std::string Callee, Data_Tree data_type, std::vector<Data_Tree> &args_type,
                 Value *scope_struct, std::vector<std::unique_ptr<ExprAST>> &Args, std::vector<Value*> &ArgsV) {
    auto *ty = get_type_from_data(args_type[0]);
    Value *v = ArgsV[0];

    llvm::Function* ctz = llvm::Intrinsic::getDeclaration(
        TheModule.get(),
        llvm::Intrinsic::cttz,
        ty
    );

    Value* zero_undef = llvm::ConstantInt::get(Builder->getInt1Ty(), 1);

    return Builder->CreateCall(ctz, {v, zero_undef});
}

Value *swap_bit(Parser_Struct parser_struct, Function *TheFunction,
                 std::string Callee, Data_Tree data_type, std::vector<Data_Tree> &args_type,
                 Value *scope_struct, std::vector<std::unique_ptr<ExprAST>> &Args, std::vector<Value*> &ArgsV) {
    auto *ty = get_type_from_data(args_type[0]);
    Value *v = ArgsV[0], *idx = ArgsV[1];
    // cast idx to same type
    Value* idx_cast = Builder->CreateIntCast(idx, ty, false);
    // 1
    Value* one = llvm::ConstantInt::get(ty, 1);
    // 1 << idx
    Value* mask = Builder->CreateShl(one, idx_cast);
    // toggle bit
    return Builder->CreateXor(v, mask);
}
Data_Tree swap_bit_ret(Parser_Struct parser_struct, std::vector<std::unique_ptr<ExprAST>>& Args) {
    return Args[0]->GetDataTree();
}

Data_Tree simd_load_ret(Parser_Struct parser_struct, std::vector<std::unique_ptr<ExprAST>>& Args) {
  Data_Tree dt = Data_Tree("vec");
  auto stmt_1 = dynamic_cast<IntExprAST*>(Args[1].get());
  auto stmt_2 = dynamic_cast<IntExprAST*>(Args[2].get());
  dt.Nested_Data.push_back(Data_Tree(data_type_to_name[stmt_1->Val]));
  dt.Nested_Data.push_back(Data_Tree(std::to_string(stmt_2->Val)));
  return dt;
}

Data_Tree vec_make_ret(Parser_Struct parser_struct, std::vector<std::unique_ptr<ExprAST>>& Args) {
  Data_Tree dt = Data_Tree("vec");
  auto stmt_2 = dynamic_cast<IntExprAST*>(Args[1].get());
  dt.Nested_Data.push_back(Args[0]->GetDataTree());
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


Value *vec_make(Parser_Struct parser_struct, Function *TheFunction,
                 std::string Callee, Data_Tree data_type, std::vector<Data_Tree> &args_type,
                 Value *scope_struct, std::vector<std::unique_ptr<ExprAST>> &Args, std::vector<Value*> &ArgsV) {
    int size;
    llvm::Type *ty;
    if (auto num_expr = dynamic_cast<IntExprAST*>(Args[1].get()))
            size = num_expr->Val;
    else
        LogError(parser_struct.line, "Vec expected size");
    
    Value *ret = Builder->CreateVectorSplat(size, ArgsV[0]);
    // ret->print(llvm::errs());
    // llvm::errs() << "\n";
    return ret;
}


Value *simd_equal(std::unique_ptr<ExprAST> &LHS, std::unique_ptr<ExprAST> &RHS, Value *L, Value *R) {
    auto *cmp = Builder->CreateICmpEQ(L, R);
    auto *vecTy = get_type_from_data(LHS->GetDataTree());
    return Builder->CreateSExt(cmp, vecTy);
}
Value *simd_and(std::unique_ptr<ExprAST> &LHS, std::unique_ptr<ExprAST> &RHS, Value *L, Value *R) {
    auto *cmp = Builder->CreateAnd(L, R);
    auto *vecTy = get_type_from_data(LHS->GetDataTree());
    return Builder->CreateSExt(cmp, vecTy);
}
Value *simd_or(std::unique_ptr<ExprAST> &LHS, std::unique_ptr<ExprAST> &RHS, Value *L, Value *R) {
    auto *cmp = Builder->CreateOr(L, R);
    auto *vecTy = get_type_from_data(LHS->GetDataTree());
    return Builder->CreateSExt(cmp, vecTy);
}

Value *vec_movemask(Parser_Struct parser_struct, Function *TheFunction,
                 std::string Callee, Data_Tree data_type, std::vector<Data_Tree> &args_type,
                 Value *scope_struct, std::vector<std::unique_ptr<ExprAST>> &Args, std::vector<Value*> &ArgsV) {
    auto *vecTy = llvm::FixedVectorType::get(Builder->getInt8Ty(), 32);

    // shift right to keep MSB
    Value *shift = Builder->CreateLShr(ArgsV[0],
        ConstantVector::getSplat(
            llvm::ElementCount::getFixed(32),
            Builder->getInt8(7)));

    // truncate to i1
    auto *i1vec = llvm::FixedVectorType::get(Builder->getInt1Ty(), 32);
    Value *trunc = Builder->CreateTrunc(shift, i1vec);

    // pack bits
    return Builder->CreateBitCast(trunc, Builder->getInt32Ty());
}


extern "C" int print_vec_i8(int8_t *v, int size) {
    printf("vec<i8,%d>\n", size);
    printf("[%d", v[0]);
    for (int i = 1; i < size; i++)
        printf(", %d ", v[i]);
    printf("]\n");
    return 0;
}
extern "C" int print_vec_i16(int16_t *v, int size) {
    printf("vec<i16,%d>\n", size);
    printf("[%d", v[0]);
    for (int i = 1; i < size; i++)
        printf(", %d ", v[i]);
    printf("]\n");
    return 0;
}
extern "C" int print_vec_int(int *v, int size) {
    printf("vec<int,%d>\n", size);
    printf("[%d", v[0]);
    for (int i = 1; i < size; i++)
        printf(", %d ", v[i]);
    printf("]\n");
    return 0;
}
extern "C" int print_vec_i64(int64_t *v, int size) {
    printf("vec<i64,%d>\n", size);
    printf("[%d", v[0]);
    for (int i = 1; i < size; i++)
        printf(", %d ", v[i]);
    printf("]\n");
    return 0;
}
extern "C" int print_vec_float(float *v, int size) {
    printf("vec<float,%d>\n", size);
    printf("[%d", v[0]);
    for (int i = 1; i < size; i++)
        printf(", %d ", v[i]);
    printf("]\n");
    return 0;
}

Value *vec_print(Parser_Struct parser_struct, Function *TheFunction,
                 std::string Callee, Data_Tree data_type, std::vector<Data_Tree> &args_type,
                 Value *scope_struct, std::vector<std::unique_ptr<ExprAST>> &Args, std::vector<Value*> &ArgsV) {

    Data_Tree dt = args_type[0];
    const std::string &vec_type = dt.Nested_Data[0].Type;
    int vec_size = std::stoi(dt.Nested_Data[1].Type);
    
    llvm::Type *vecTy = get_type_from_data(dt);
    Value *alloca = CreateEntryBlockAlloca(TheFunction, "vec", vecTy);

    Builder->CreateStore(ArgsV[0], alloca);

    Value *ptr = Builder->CreateBitCast(alloca, int8PtrTy);

    call("print_vec_"+vec_type, {ptr, const_int(vec_size)});
    return const_int(0);
}
