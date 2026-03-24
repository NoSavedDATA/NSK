#pragma once


#include <iostream>
#include <filesystem>
#include <fstream>
#include <stack>
#include <string>
#include <map>
#include <memory>
#include <vector>
#include <unordered_map>

#include "tokenizer_if.h"

struct TokenizerSTD : TokenizerIF {
    
    public:
        TokenizerSTD(std::string);
        int getToken() override;
};

enum TokenizerSTDTokens {
    std_tok_eof = -1,
    std_tok_number = -2,
    std_tok_int = -12,
    std_tok_str = -3,
    std_tok_char = -4,
    std_tok_identifier = -11,
    std_tok_arrow = -5,
    std_tok_equal = -6,
    std_tok_diff = -7,
    std_tok_higher_eq = -8,
    std_tok_minor_eq = -9,
    std_tok_int_div = -10,
    std_tok_space = 10
};
