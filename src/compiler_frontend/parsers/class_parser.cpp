#include <ctype.h>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <stack>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include "../../runtime/common/extension_functions.h"
#include "../../runtime/compiler_frontend/logging_v.h"
#include "../include.h"




namespace fs = std::filesystem;

static void ParseImport(std::unique_ptr<TokenizerClass> &class_tokenizer) {
    int tok = class_tokenizer->getToken();

    std::string lib_name = class_tokenizer->IdentifierStr;
    tok = class_tokenizer->getToken();
    while (tok=='.') {
        tok = class_tokenizer->getToken();
        lib_name += "/" + class_tokenizer->IdentifierStr;
        tok = class_tokenizer->getToken();
    }

    std::string full_path_lib = class_tokenizer->dir+"/"+lib_name+".nk";

    std::string lib_path = std::getenv("NSK_LIBS");
    std::string as_include = lib_path + "/" + lib_name + "/include.nk";

    if (fs::exists(full_path_lib)) {
        ParseClasses(full_path_lib);
    } else if (fs::exists(as_include)) {
        ParseClasses(as_include);
    } else {
        LogErrorC(-1, "Could not find library " + lib_name);
    }
}

static void ParseClass(std::unique_ptr<TokenizerClass> &class_tokenizer) {
    class_tokenizer->getToken();
    std::string _class = class_tokenizer->IdentifierStr;
    Classes[_class] = 1;
}

void ParseClasses(std::string fname) {
    std::unique_ptr<TokenizerClass> class_tokenizer;
    class_tokenizer = std::make_unique<TokenizerClass>(fname);
    
    int tok = 0;
    while (tok!=class_tok_eof) {
        tok = class_tokenizer->getToken();
        switch (tok) {
            case(class_tok_import):
                ParseImport(class_tokenizer);
                break;
            case(class_tok_class):
                ParseClass(class_tokenizer);
                break;
            default:
                break;
        }
    }
}

