#pragma once
#include "../nsk_cpp_llvm.h"

 

Value *simd_equal(std::unique_ptr<ExprAST> &LHS, std::unique_ptr<ExprAST> &RHS, Value *L, Value *R);
Value *simd_and(std::unique_ptr<ExprAST> &LHS, std::unique_ptr<ExprAST> &RHS, Value *L, Value *R);
Value *simd_or(std::unique_ptr<ExprAST> &LHS, std::unique_ptr<ExprAST> &RHS, Value *L, Value *R);
