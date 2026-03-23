#pragma once


extern std::vector<std::unique_ptr<FunctionAST>> AllFunctions;

void HandleExtern();

void HandleImport();

void HandleClass(); 

void HandleDefinition(); 

void HandleTopLevelExpression(); 
void CodegenTopLevelExpression(std::unique_ptr<FunctionAST> &FnAST);


void InitializeTokenizer(); 
 
void MainLoop(); 

extern "C" float putchard(float X); 

extern "C" float printd(float X); 


void build_dicts();
