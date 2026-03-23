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


struct TokenizerIF {
    // std::stack<std::string> dirs, files;
    int line, NumVal, LastSeenTabs, SeenTabs;
    std::istream* current;
    std::string current_dir, current_file, IdentifierStr;
    char cur_c=' ', LastChar=' ';
    
    public:
        TokenizerIF(std::string);

        char get();
        std::istream& get_word();
        bool openFile(std::string);
        bool importFile(std::string, int);
        int getToken();
};

enum TokenizerIFTokens {
    if_tok_eof = -1,
    if_tok_number = -2,
    if_tok_int = -12,
    if_tok_str = -3,
    if_tok_char = -4,
    if_tok_identifier = -11,
    if_tok_arrow = -5,
    if_tok_equal = -6,
    if_tok_diff = -7,
    if_tok_higher_eq = -8,
    if_tok_minor_eq = -9,
    if_tok_int_div = -10,
    if_tok_space = 10
};
