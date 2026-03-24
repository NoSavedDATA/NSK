#include "../src/compiler_frontend/tokenizers/include.h"
#include "data_ops.h"
#include "std_parser.h"


STD_FN_Expr::STD_FN_Expr(std::string name) {

    if (begins_with(name, "DT") && ends_with(name, "_Create")) {
      std::string dt = remove_substring(name, "_Create");
      line = "\tstruct_create_fn[\"" + name + "\"] = " + name + ";\n";
    } else
      line = "\tllvm_callee[\"" + name + "\"] = " + name + ";\n";
}

std::string STD_FN_Expr::GetLine(std::string lines) {
    return line;
}

void ParseSTDFN(TokenizerSTD &tokenizer, std::vector<STD_FN_Expr> &std_fn) {
    int CurTok = tokenizer.getToken(); // eat *

    if (CurTok!=std_tok_identifier)
        return;
    std::string fn_name = tokenizer.IdentifierStr;
    CurTok = tokenizer.getToken(); // eat *
    if(CurTok!='(')
        return;
    std_fn.push_back(STD_FN_Expr(fn_name));
}


std::vector<STD_FN_Expr> ParseSTD(TokenizerSTD &tokenizer) {
    int CurTok = 0;
    std::vector<STD_FN_Expr> std_fn;
    do {
        CurTok = tokenizer.getToken();

        if (CurTok==std_tok_identifier) {
            if (tokenizer.IdentifierStr=="Value") {
                CurTok = tokenizer.getToken();
                if (CurTok=='*')
                    ParseSTDFN(tokenizer, std_fn);
            }
        }

    } while(CurTok!=std_tok_eof);
    return std::move(std_fn);
}

std::vector<STD_FN_Expr> BuildSTD() {
    TokenizerSTD tokenizer = TokenizerSTD("src/std/codegen.h");
    return ParseSTD(tokenizer);
}
