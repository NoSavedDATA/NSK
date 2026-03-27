#include "../nsk_cpp_llvm.h"


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

