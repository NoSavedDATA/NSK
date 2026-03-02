#pragma once
#include <cstddef>

#include "../mangler/scope_struct.h"


extern "C" char *GetEmptyChar(Scope_Struct *scope_struct);


extern "C" void FreeCharFromFunc(char *_char, char *func); 


extern "C" void FreeChar(char *_char); 


extern "C" char *CopyString(Scope_Struct *, char *in_str);


extern "C" char * ConcatStr(Scope_Struct *scope_struct, char *lc, char *rc);

extern "C" char * ConcatStrFreeLeft(Scope_Struct *scope_struct, char *lc, char *rc);


extern "C" char * ConcatFloatToStr(Scope_Struct *scope_struct, char *lc, float r);

extern "C" char * ConcatNumToStrFree(Scope_Struct *scope_struct, char *lc, float r);



// inline char *CopyStringI(Scope_Struct *scope_struct, char *in_str)
// {
//   size_t length = strlen(in_str) + 1;

//   char *copied = allocate<char>(scope_struct, length, "str");

//   memcpy(copied, in_str, length);
//   copied[length] = '\0';

//   return copied;
// }
