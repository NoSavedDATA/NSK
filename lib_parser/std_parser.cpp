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


void ParseSTD(TokenizerSTD &tokenizer, std::vector<STD_FN_Expr> &std_fn) {
    int CurTok = 0;
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
}

std::vector<STD_FN_Expr> BuildSTD() {
    std::vector<STD_FN_Expr> std_fn;

    TokenizerSTD tokenizer = TokenizerSTD("src/std/codegen.h");
    ParseSTD(tokenizer, std_fn);
    TokenizerSTD tokenizer_simd = TokenizerSTD("src/simd/codegen.h");
    ParseSTD(tokenizer_simd, std_fn);
    

    return std::move(std_fn);
}
