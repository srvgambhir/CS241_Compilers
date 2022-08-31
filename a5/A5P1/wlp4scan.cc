#include <iostream>
#include <string>
#include <vector>
#include "scanner.h"

// wlp4 language scanner

int main() {
  std::string line;
  std::vector<std::vector<Token>> program;
  try {
	  while (getline(std::cin, line)) {
		  std::vector<Token> tokenLine = scan(line);
		  program.push_back(tokenLine);
    
	  }
	  
	  for (auto &tokenLine : program) {
		  for (auto &token : tokenLine) {      	 	
			  std::cout << token << std::endl;
		  }
	  }
  
  } catch (ScanningFailure &f) {
    std::cerr << f.what() << std::endl;
    return 1;
  }
  
  return 0;
}
