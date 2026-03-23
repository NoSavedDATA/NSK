#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Target/TargetMachine.h"


#include "src/KaleidoscopeJIT.h"

#include <algorithm>
#include <cstdarg>
#include <cassert>
#include <cctype>
#include <cstring>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <iostream>
#include <numeric>
#include <utility>
#include <vector>
#include <iomanip>
#include <math.h>
#include <fenv.h>
#include <tuple>
#include <chrono>
#include <thread>
#include <random>
#include <float.h>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "src/include.h"


using namespace llvm;
using namespace llvm::orc;


std::map<std::string, int> NotatorsMap = {
  {"bias", bias},
  {"fp32", fp32},
  {"fp16", fp16},
  {"causal", causal},
};


LCG rng(generate_custom_seed());


  // Error Colors
// \033[0m default
// \033[31m red
// \033[33m yellow
// \033[34m blue
// \033[95m purple


//===----------------------------------------------------------------------===//
// Code Generation
//===----------------------------------------------------------------------===//

//global

std::vector<std::unique_ptr<FunctionAST>> AllFunctions;

// Vars
std::map<std::string, std::vector<char *>> ClassStrVecs;
std::map<std::string, float> NamedClassValues;
std::map<std::string, int> NamedInts;
std::map<std::string, std::vector<std::pair<std::string, std::string>>> ScopeVarsToClean;
std::map<std::string, char *> ScopeNamesToClean;
std::map<int, std::map<std::string, std::vector<std::string>>> ThreadedScopeTensorsToClean;






// File Handling
std::vector<char *> glob_str_files;




//===----------------------------------------------------------------------===//
// Top-Level parsing and JIT Driver
//===----------------------------------------------------------------------===//

static void HandleExtern() {
  Parser_Struct parser_struct;
  if (auto ProtoAST = ParseExtern(parser_struct)) {
    if (auto *FnIR = ProtoAST->codegen()) {
      fprintf(stderr, "Read extern: ");
      FnIR->print(errs());
      fprintf(stderr, "\n");
      FunctionProtos[ProtoAST->getName()] = std::move(ProtoAST);
    }
  } else {
    // Skip token for error recovery.
    getNextToken();
  }
}

static void HandleImport() {
    Parser_Struct parser_struct;
    parser_struct.line = LineCounter;
    ParseImport(parser_struct);
}

static void HandleClass() {
    Parser_Struct parser_struct;
    parser_struct.line = LineCounter;
    ParseClass(parser_struct);
}

static void HandleDefinition() {
  
  Parser_Struct parser_struct;
  if (auto FnAST = ParseDefinition(parser_struct)) {

    FunctionProtos[FnAST->getProto().getName()] =
      std::make_unique<PrototypeAST>(FnAST->getProto());

    ExitOnErr(TheJIT->addAST(std::move(FnAST)));
    // AllFunctions.push_back(std::move(FnAST));
  } else {
    // Skip token for error recovery.
    getNextToken();
  }
}

void linkExecutable() {
    // std::string out = R"(ld.lld-19 \
    //   /usr/lib/x86_64-linux-gnu/crt1.o \
    //   /usr/lib/x86_64-linux-gnu/crti.o \
    //   /usr/lib/gcc/x86_64-linux-gnu/9/crtbegin.o \
    //   output.o \
    //   --start-group static/runtime.a --end-group \
    //   -L/usr/lib/gcc/x86_64-linux-gnu/9 -lstdc++ \
    //   -L/lib/x86_64-linux-gnu -lgcc_s -lgcc -lc -lm \
    //   /usr/lib/gcc/x86_64-linux-gnu/9/crtend.o \
    //   /usr/lib/x86_64-linux-gnu/crtn.o \
    //   --dynamic-linker /lib64/ld-linux-x86-64.so.2 \
    //   -o output)";
    
    // std::string out = "clang++-19 -no-pie output.o static/runtime.a -o output";
    std::string out = "clang++-19 -static -static-libstdc++ -static-libgcc \
    output.o static/runtime.a -o output";
    std::cout << "" << out << "\n";
    system(out.c_str());
}

static void Compile() {
    std::cout << "compile " << "\n";
    for (auto &FuncAST : AllFunctions)
        FuncAST->codegen();
    
    std::error_code EC;
    raw_fd_ostream dest("output.o", EC, sys::fs::OF_None);

    legacy::PassManager pass;

    if (CTM->addPassesToEmitFile(pass, dest, nullptr,
                                 llvm::CodeGenFileType::ObjectFile)) {
        errs() << "TargetMachine can't emit object file\n";
        return;
    }

    pass.run(*TheModule);
    dest.flush();
    linkExecutable();
}


static void CodegenTopLevelExpression(std::unique_ptr<FunctionAST> &FnAST) {


    // JIT Version
    auto *FnIR =  FnAST->codegen();
    // Create a ResourceTracker for memory managment
    // anonymous expression -- that way we can free it after executing.
    auto RT = TheJIT->getMainJITDylib().createResourceTracker();
    auto TSM = ThreadSafeModule(std::move(TheModule), std::move(TheContext));
    ExitOnErr(TheJIT->addModule(std::move(TSM), RT));
    // Add IR module

    InitializeModule();
    // Points __anon_expr
    auto Sym = ExitOnErr(TheJIT->lookup("__anon_expr"));
    // Get the symbol's address and cast it to the right type (takes no
    // arguments, returns a float) so we can call it as a native function.
    auto *FP = Sym.getAddress().toPtr<float (*)()>();
    auto fp = FP();
    // fprintf(stderr, "%.2f\n", fp);
    // Delete the anonymous expression module from the JIT.
    ExitOnErr(RT->remove());    
}



static void HandleTopLevelExpression() {
  // Evaluate a top-level expression into an anonymous function.
  
  Parser_Struct parser_struct;
  parser_struct.function_name = "__anon_expr";

  if (std::unique_ptr<FunctionAST> FnAST = ParseTopLevelExpr(parser_struct)) {
    CodegenTopLevelExpression(std::ref(FnAST));	
  
  } else {
    // Skip token for error recovery.
    getNextToken();
  }
}


void InitializeTokenizer() {

    if (Sys_Arguments.size()>0)
    {
        tokenizer.openFile(Sys_Arguments[0]);
        getNextToken();

    } else
        getNextToken();
}

/// top ::= definition | external | expression | ';'
static void MainLoop() {
    while (true) {
         // std::cout << "MAIN LOOP, reading token: " << CurTok << "/" << ReverseToken(CurTok) << "\n";
        switch (CurTok) {
        case 13:
            std::cout << "FOUND CARRIAGE RETURN" << ".\n";
            break;
        case tok_eof:
            return;
        case ';': // ignore top-level semicolons.
            getNextToken();
            break;
        case '.': 
            getNextToken();
            break;
        case tok_space:
            getNextToken();
            break;
        case tok_tab:
            getNextToken();
            break;
        case tok_def:
            HandleDefinition();
            break;
        case tok_class:
            HandleClass();
            break;
        case tok_import:
            HandleImport();
            break;
        case tok_extern:
            HandleExtern();
            break;
        case tok_constructor:
            LogErrorNextBlock(LineCounter, "Constructor has no class associated.");
            break;
        default:
            // std::cout << "Wait top level" <<  ".\n";
            // std::cout << "reading token: " << CurTok << "/" << ReverseToken(CurTok) << "\n";
            HandleTopLevelExpression(); 
            // std::cout << "Finished top level" <<  ".\n";
            break;
        }
    }
}


//===----------------------------------------------------------------------===//
// "Library" functions that can be "extern'd" from user code.
//===----------------------------------------------------------------------===//

/// putchard - putchar that takes a float and returns 0.
extern "C" float putchard(float X) {
  fputc((char)X, stderr);
  return 0;
}

/// printd - printf that takes a float prints it as "%f\n", returning 0.
extern "C" float printd(float X) {
  fprintf(stderr, "%f\n", X);
  return 0;
}

//===----------------------------------------------------------------------===//
// Main driver code.
//===----------------------------------------------------------------------===//

__attribute__((constructor))
void early_init() {
    // std::cout << "Constructor Function Executed\n";
  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter(); // Prepare for target hardware
  InitializeNativeTargetAsmParser();
}

int main(int argc, char* argv[]) {

    for (int i = 1; i < argc; i++) {
        // std::cout << "Argument " << i << ": " << argv[i] << "\n";
        Sys_Arguments.push_back(argv[i]);
        if(i==1) {
           tokenizer.files.pop();
           tokenizer.files.push(argv[i]);
        }
    }



  // DT_charv
  struct_create_fn["DT_charv"] = DT_charv_create;
  Function_Arg_DataTypes["charv_Create"]["0"] = Data_Tree("Scope_Struct");
  Function_Arg_DataTypes["charv_Create"]["1"] = Data_Tree("int");
  Function_Arg_Names["charv_Create"] = {"0", "1"};

  // DT_vec
  struct_create_fn["DT_vec"] = DT_vec_create;
  Function_Arg_DataTypes["vec_Create"]["0"] = Data_Tree("Scope_Struct");
  Function_Arg_DataTypes["vec_Create"]["1"] = Data_Tree("int");
  Function_Arg_DataTypes["vec_Create"]["2"] = Data_Tree("int");
  Function_Arg_Names["vec_Create"] = {"0", "1", "2"};



  // c_open
  functions_return_data_type["c_open"] = Data_Tree("int");
  llvm_callee["c_open"] = c_open;
  Function_Arg_DataTypes["c_open"]["0"] = Data_Tree("Scope_Struct");
  Function_Arg_DataTypes["c_open"]["1"] = Data_Tree("str");
  Function_Arg_Names["c_open"] = {"0", "1"};
  Function_Required_Arg_Count["c_open"] = 1;

  // c_read
  functions_return_data_type["c_read"] = Data_Tree("i64");
  llvm_callee["c_read"] = c_read;
  Function_Arg_DataTypes["c_read"]["0"] = Data_Tree("Scope_Struct");
  Function_Arg_DataTypes["c_read"]["1"] = Data_Tree("int");
  Function_Arg_DataTypes["c_read"]["2"] = Data_Tree("charv");
  Function_Arg_DataTypes["c_read"]["3"] = Data_Tree("int");
  Function_Arg_Names["c_read"] = {"0", "1", "2", "3"};
  Function_Required_Arg_Count["c_read"] = 3;


  // c_memchr
  functions_return_data_type["c_memchr"] = Data_Tree("i64");
  llvm_callee["c_memchr"] = c_memchr;
  Function_Arg_DataTypes["c_memchr"]["0"] = Data_Tree("Scope_Struct");
  Function_Arg_DataTypes["c_memchr"]["1"] = Data_Tree("str");
  Function_Arg_DataTypes["c_memchr"]["2"] = Data_Tree("char");
  Function_Arg_DataTypes["c_memchr"]["3"] = Data_Tree("i64");
  Function_Arg_Names["c_memchr"] = {"0", "1", "2", "3"};
  Function_Required_Arg_Count["c_memchr"] = 3;

  // c_strlen
  functions_return_data_type["c_strlen"] = Data_Tree("i64");
  llvm_callee["c_strlen"] = c_strlen;
  Function_Arg_DataTypes["c_strlen"]["0"] = Data_Tree("Scope_Struct");
  Function_Arg_DataTypes["c_strlen"]["1"] = Data_Tree("str");
  Function_Arg_Names["c_strlen"] = {"0", "1"};
  Function_Required_Arg_Count["c_strlen"] = 1;


  // c_memcpy
  functions_return_data_type["c_memcpy"] = Data_Tree("int");
  llvm_callee["c_memcpy"] = c_memcpy;
  Function_Arg_DataTypes["c_memcpy"]["0"] = Data_Tree("Scope_Struct");
  Function_Arg_DataTypes["c_memcpy"]["1"] = Data_Tree("str");
  Function_Arg_DataTypes["c_memcpy"]["2"] = Data_Tree("charv");
  Function_Arg_DataTypes["c_memcpy"]["3"] = Data_Tree("i64");
  Function_Arg_Names["c_memcpy"] = {"0", "1", "2", "3"};
  Function_Required_Arg_Count["c_memcpy"] = 3;


  // str_set
  functions_return_data_type["str_set"] = Data_Tree("int");
  llvm_callee["str_set"] = str_set;
  Function_Arg_DataTypes["str_set"]["0"] = Data_Tree("Scope_Struct");
  Function_Arg_DataTypes["str_set"]["1"] = Data_Tree("str");
  Function_Arg_DataTypes["str_set"]["2"] = Data_Tree("int");
  Function_Arg_DataTypes["str_set"]["3"] = Data_Tree("int");
  Function_Arg_Names["str_set"] = {"0", "1", "2", "3"};
  Function_Required_Arg_Count["str_set"] = 3;

  // str_offset
  functions_return_data_type["str_offset"] = Data_Tree("str");
  llvm_callee["str_offset"] = str_offset;
  Function_Arg_DataTypes["str_offset"]["0"] = Data_Tree("Scope_Struct");
  Function_Arg_DataTypes["str_offset"]["1"] = Data_Tree("str");
  Function_Arg_DataTypes["str_offset"]["2"] = Data_Tree("int");
  Function_Arg_Names["str_offset"] = {"0", "1", "2"};
  Function_Required_Arg_Count["str_offset"] = 2;

  // err
  functions_return_data_type["err"] = Data_Tree("int");
  llvm_callee["err"] = err;
  Function_Arg_DataTypes["err"]["0"] = Data_Tree("Scope_Struct");
  Function_Arg_DataTypes["err"]["1"] = Data_Tree("str");
  Function_Arg_Names["err"] = {"0", "1"};
  Function_Required_Arg_Count["err"] = 1;

  // _malloc
  functions_return_data_type["_malloc"] = Data_Tree("any");
  llvm_callee["_malloc"] = _malloc;
  Function_Arg_DataTypes["_malloc"]["0"] = Data_Tree("Scope_Struct");
  Function_Arg_DataTypes["_malloc"]["1"] = Data_Tree("int");
  Function_Arg_Names["_malloc"] = {"0", "1"};
  Function_Required_Arg_Count["_malloc"] = 1;

  // allocate
  functions_return_data_type["alloc"] = Data_Tree("any");
  llvm_callee["alloc"] = alloc;
  Function_Arg_DataTypes["alloc"]["0"] = Data_Tree("Scope_Struct");
  Function_Arg_DataTypes["alloc"]["1"] = Data_Tree("int");
  Function_Arg_DataTypes["alloc"]["2"] = Data_Tree("str");
  Function_Arg_Names["alloc"] = {"0", "1", "2"};
  Function_Required_Arg_Count["alloc"] = 2;
  
  // i8
  functions_return_data_type["i8"] = Data_Tree("i8");
  llvm_callee["i8"] = parse_i8;
  Function_Arg_DataTypes["i8"]["0"] = Data_Tree("Scope_Struct");
  Function_Arg_DataTypes["i8"]["1"] = Data_Tree("any");
  Function_Arg_Names["i8"] = {"0", "1"};
  Function_Required_Arg_Count["i8"] = 1;
  // i16
  functions_return_data_type["i16"] = Data_Tree("i16");
  llvm_callee["i16"] = parse_i16;
  Function_Arg_DataTypes["i16"]["0"] = Data_Tree("Scope_Struct");
  Function_Arg_DataTypes["i16"]["1"] = Data_Tree("any");
  Function_Arg_Names["i16"] = {"0", "1"};
  Function_Required_Arg_Count["i16"] = 1;
  // int
  functions_return_data_type["int"] = Data_Tree("int");
  llvm_callee["int"] = parse_int;
  Function_Arg_DataTypes["int"]["0"] = Data_Tree("Scope_Struct");
  Function_Arg_DataTypes["int"]["1"] = Data_Tree("any");
  Function_Arg_Names["int"] = {"0", "1"};
  Function_Required_Arg_Count["int"] = 1;
  // i64
  functions_return_data_type["i64"] = Data_Tree("i64");
  llvm_callee["i64"] = parse_i64;
  Function_Arg_DataTypes["i64"]["0"] = Data_Tree("Scope_Struct");
  Function_Arg_DataTypes["i64"]["1"] = Data_Tree("any");
  Function_Arg_Names["i64"] = {"0", "1"};
  Function_Required_Arg_Count["i64"] = 1;

  // ctz
  functions_return_data_type["ctz"] = Data_Tree("int");
  llvm_callee["ctz"] = ctz;
  Function_Arg_DataTypes["ctz"]["0"] = Data_Tree("Scope_Struct");
  Function_Arg_DataTypes["ctz"]["1"] = Data_Tree("any");
  Function_Arg_Names["ctz"] = {"0", "1"};
  Function_Required_Arg_Count["ctz"] = 1;

  // swap_bit
  function_return_overwrite["swap_bit"] = swap_bit_ret;
  llvm_callee["swap_bit"] = swap_bit;
  Function_Arg_DataTypes["swap_bit"]["0"] = Data_Tree("Scope_Struct");
  Function_Arg_DataTypes["swap_bit"]["1"] = Data_Tree("any");
  Function_Arg_DataTypes["swap_bit"]["2"] = Data_Tree("int");
  Function_Arg_Names["swap_bit"] = {"0", "1", "2"};
  Function_Required_Arg_Count["swap_bit"] = 2;

  // simd_load
  function_return_overwrite["simd_load"] = simd_load_ret;
  llvm_callee["simd_load"] = simd_load;
  Function_Arg_DataTypes["simd_load"]["0"] = Data_Tree("Scope_Struct");
  Function_Arg_DataTypes["simd_load"]["1"] = Data_Tree("any");
  Function_Arg_DataTypes["simd_load"]["2"] = Data_Tree("int");
  Function_Arg_DataTypes["simd_load"]["3"] = Data_Tree("int");
  Function_Arg_Names["simd_load"] = {"0", "1", "2", "3"};
  Function_Required_Arg_Count["simd_load"] = 3;

  // vec_make
  function_return_overwrite["vec_make"] = vec_make_ret;
  llvm_callee["vec_make"] = vec_make;
  Function_Arg_DataTypes["vec_make"]["0"] = Data_Tree("Scope_Struct");
  Function_Arg_DataTypes["vec_make"]["1"] = Data_Tree("any");
  Function_Arg_DataTypes["vec_make"]["2"] = Data_Tree("int");
  Function_Arg_Names["vec_make"] = {"0", "1", "2"};
  Function_Required_Arg_Count["vec_make"] = 2;

  // vec_movemask
  functions_return_data_type["vec_movemask"] = Data_Tree("int");
  llvm_callee["vec_movemask"] = vec_movemask;
  Function_Arg_DataTypes["vec_movemask"]["0"] = Data_Tree("Scope_Struct");
  Function_Arg_DataTypes["vec_movemask"]["1"] = Data_Tree("any");
  Function_Arg_Names["vec_movemask"] = {"0", "1"};
  Function_Required_Arg_Count["vec_movemask"] = 1;

  // vec_print
  functions_return_data_type["vec_print"] = Data_Tree("int");
  llvm_callee["vec_print"] = vec_print;
  Function_Arg_DataTypes["vec_print"]["0"] = Data_Tree("Scope_Struct");
  Function_Arg_DataTypes["vec_print"]["1"] = Data_Tree("vec");
  Function_Arg_Names["vec_print"] = {"0", "1"};
  Function_Required_Arg_Count["vec_print"] = 1;




  // print
  functions_return_data_type["print"] = Data_Tree("void");
  llvm_callee["print"] = print;

  Function_Arg_DataTypes["print"]["0"] = Data_Tree("Scope_Struct");
  Function_Arg_DataTypes["print"]["1"] = Data_Tree("str");
  Function_Arg_DataTypes["print"]["2"] = Data_Tree("str");
  Function_Arg_DataTypes["print"]["3"] = Data_Tree("str");
  Function_Arg_DataTypes["print"]["4"] = Data_Tree("str");
  Function_Arg_DataTypes["print"]["5"] = Data_Tree("str");
  Function_Arg_DataTypes["print"]["6"] = Data_Tree("str");
  Function_Arg_DataTypes["print"]["7"] = Data_Tree("str");
  Function_Arg_DataTypes["print"]["8"] = Data_Tree("str");
  Function_Arg_DataTypes["print"]["9"] = Data_Tree("str");
  Function_Arg_DataTypes["print"]["10"] = Data_Tree("str");
  Function_Arg_Names["print"] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10"};

  set_functions_return_type();
  set_functions_args_type();
  set_user_functions();

  // Prime the first token.
  InitializeTokenizer();
  prebuild();

  TheJIT = ExitOnErr(KaleidoscopeJIT::Create());
  InitializeModule();
  MainLoop();
  // Compile();
  return 0;
}

