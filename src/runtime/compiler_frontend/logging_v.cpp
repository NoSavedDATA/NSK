#include <fstream>
#include <iostream>
#include <string>

#include "global_vars.h"


void LogErrorC(int line, std::string Str) {


  if(line!=-1)
  {
    if (CurrentFile!="main") {
      std::cout << "\n\n" << CurrentFile << "\n";

      
      
      std::ifstream file;
      std::string str_line;
      
      int l=0;
      file.open(CurrentFile);
      while(l<line&&std::getline(file, str_line))
        l++;

      file.close();
      printf("%s", str_line.c_str());
    }

    std::cout << "\nLine: " << line << "\n   ";
  } else
    std::cout << "\n\n" << CurrentFile << "\n";


  std::cout << "\033[31m Error: \033[0m " << Str << "\n\n";  

  Shall_Exit = true;
}
