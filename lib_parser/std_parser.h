#pragma once
#include <string>

struct STD_FN_Expr {
    std::string line;
    STD_FN_Expr(std::string);
    std::string GetLine(std::string);
};

std::vector<STD_FN_Expr> BuildSTD();
