#include <iostream>
#include <string>
#include <vector>
#include "scanner.h"
#include <memory>
#include <map>


class AssemblyFailure {
    std::string message;

  public:
    AssemblyFailure(std::string message) : message(std::move(message)) {}

    // Returns the message associated with the exception.
    const std::string &what() const { return message; }
};

// R-format instructions, store the instruction names and op codes
std::map<std::string, int64_t> r_form {
	{ "add", 32 },
	{ "sub", 34 },
	{ "mult", 24 },
	{ "multu", 25 },
	{ "div", 26 },
	{ "divu", 27 },
	{ "mfhi", 16 },
	{ "mflo", 18 },
	{ "lis", 20 },
	{ "slt", 42 },
	{ "sltu", 43 },
	{ "jr", 8 },
	{ "jalr", 9 }
};

// I-format instructions, store the instrucion names and op codes
std::map<std::string,int64_t> i_form {
	{ "lw", 35 },
	{ "sw", 43 },
	{ "beq", 4 },
	{ "bne", 5 }
};

//bounds for word operands
const int64_t MAX_W = 4294967295;
const int64_t LOW_W = -2147483648;

//bounds for I-format immediates
const int64_t MAX_D = 32767;
const int64_t LOW_D = -32768;
const int64_t MAX_H = 65535;

//bounds for register values
const int64_t MAX_R = 31;
const int64_t LOW_R = 0;


// function to assemble .word instruction
void print_w(int64_t i) {
	char c = i >> 24;
	std::cout << c;
	c = i >> 16;
	std::cout << c;
	c = i >> 8;
	std::cout << c;
	c = i;
	std::cout << c;
}

// function to print R-format instruction
void print_r(int64_t func, int64_t d, int64_t s, int64_t t, int64_t op) {
	int64_t word = (func << 26) | (s << 21) | (t << 16) | (d << 11) | op;
	char c = (word >> 24) & 0xff;
	std::cout << c;
	c = (word >> 16) & 0xff;
	std::cout << c;
	c = (word >> 8) & 0xff;
	std::cout << c;
	c = word & 0xff;
	std::cout << c;
}

// function to print I-format instruction
void print_i(int64_t op, int64_t s, int64_t t, int64_t i) {
	int64_t word = (op << 26) | (s << 21) | (t << 16) | (i & 0xffff);
	char c = (word >> 24) & 0xff;
	std::cout << c;
	c = (word >> 16) & 0xff;
        std::cout << c;
        c = (word >> 8) & 0xff;
        std::cout << c;
        c = word & 0xff;
        std::cout << c;
}


int main() {
  std::string input;

  // create vector storing the mips lines to be assembled (pass 1 = parsing, pass 2 = assembly)
  std::vector<std::unique_ptr<std::vector<Token>>> program;
  
  try {
    while (getline(std::cin, input)) {
	    std::vector<Token> tokenLine = scan(input);
	    auto line = std::make_unique<std::vector<Token>>(tokenLine);
      
	    program.push_back(std::move(line));
    }  
  } catch (ScanningFailure &f) {
	  std::cerr << f.what() << std::endl;
	  return 1;
  }


  std::map<std::string, int64_t> labels;
  int64_t line_count = 0;
  
  // Pass 1 - parsing
  try {
  	for (auto &line : program) {
	  	for (auto token = line->begin(); token != line->end(); ++token) {

			Token::Kind kd = token->getKind();
			std::string txt = token->getLexeme();

			// add to labels table
		  	if (kd == Token::LABEL) {
				txt = txt.substr(0, txt.size()-1);

				// label is not at start of line, and the preceding token is also not a label
				if (token != line->begin() && (token-1)->getKind() != Token::LABEL) {
					throw AssemblyFailure("ERROR");
			  	}

				// check for duplicate labels
				if (labels.count(txt)) {
					throw AssemblyFailure("ERROR: duplicate label");
				}

				labels[txt] = 4 * line_count;
		  	}

			// determine if .word instr is valid
			else if (kd == Token::WORD) {

				// check if # of operands is correct
				if ((token+1) != line->end() && (token+2) == line->end()) {

					// check if INT operand is in range
					if ((token+1)->getKind() == Token::INT) {
						int64_t x = (token+1)->toNumber();
						if (x >= LOW_W && x <= MAX_W) {
							++token;
							++line_count;
							continue;
						}
						else {
							throw AssemblyFailure("ERROR: operand out of range");
						}
					}

					// check is HEXINT operand is in range
					else if ((token+1)->getKind() == Token::HEXINT) {
						int64_t x = (token+1)->toNumber();
						if (x <= MAX_W) {
							++token;
							++line_count;
							continue;
						}
						else {
							throw AssemblyFailure("ERROR: operand out of range");
						}
					}

					else if ((token+1)->getKind() == Token::ID) {
						++token;
						++line_count;
						continue;
					}

					else {
						throw AssemblyFailure("ERROR");
					}
				}

				else {
					throw AssemblyFailure("ERROR");
				}
			}

			// determine if R-format instr is valid
			else if (r_form.count(txt)) {

				// check if R-format instructions with 3 registers are valid
				if (txt == "add" || txt == "sub" || txt == "slt" || txt == "sltu") {

					// check if there are enough tokens
					for (int i = 1; i <= 5; ++i) {
						if ((token+i) == line->end()) {
							throw AssemblyFailure("ERROR");
						}
					}

					// check Token kinds
					if ((token+1)->getKind() == Token::REG && (token+2)->getKind() == Token::COMMA && (token+3)->getKind() == Token::REG && 
							(token+4)->getKind() == Token::COMMA && (token+5)->getKind() == Token::REG && (token+6) == line->end()) {
						int64_t reg1 = (token+1)->toNumber();
						int64_t reg2 = (token+3)->toNumber();
						int64_t reg3 = (token+5)->toNumber();

						// check if registers are in range
						if ((reg1 >= LOW_R  && reg1 <= MAX_R) && (reg2 >= LOW_R && reg2 <= MAX_R) && (reg3 >= LOW_R && reg3 <= MAX_R)) {
							token += 5;
							++line_count;
							continue;
						}
						else {
							throw AssemblyFailure("ERROR: Invalid register");
						}
					}
					else {
						throw AssemblyFailure("ERROR");
					}
				}

				else if (txt == "mult" || txt == "multu" || txt == "div" || txt == "divu") {

					for (int i = 1; i <= 3; ++i) {
						if ((token+i) == line->end()) {
							throw AssemblyFailure("ERROR");
						}
					}

					if ((token+1)->getKind() == Token::REG && (token+2)->getKind() == Token::COMMA && (token+3)->getKind() == Token::REG && (token+4) == line->end()) {
						int64_t reg1 = (token+1)->toNumber();
						int64_t reg2 = (token+3)->toNumber();
						if ((reg1 >= LOW_R  && reg1 <= MAX_R) && (reg2 >= LOW_R && reg2 <= MAX_R)) {
                                                        token += 3;
                                                        ++line_count;
                                                        continue;
                                                }
						else {
							throw AssemblyFailure("ERROR: Invalid register");
						}
					}
					else {
						throw AssemblyFailure("ERROR");
					}
				}

				else if (txt == "mfhi" || txt == "mflo" || txt == "lis" || txt == "jr" || txt == "jalr") {

					if ((token+1) == line->end()) {
						throw AssemblyFailure("ERROR");
					}

					if ((token+1)->getKind() == Token::REG && (token+2) == line->end()) {
						int64_t reg1 = (token+1)->toNumber();
						if (reg1 >= LOW_R && reg1 <= MAX_R) {
							++token;
							++line_count;
							continue;
						}
						else {
							throw AssemblyFailure("ERROR: Invalid register");
						}
					}
					else {
						throw AssemblyFailure("ERROR");
					}
				}
			}

			// determine if I-format instr is valid
			else if (i_form.count(txt)) {

				// check if lw and sw commands are valid
				if (txt == "lw" || txt == "sw") {

					for (int i = 1; i <= 6; ++i) {
						if ((token+i) == line->end()) {
							throw AssemblyFailure("ERROR");
						}
					}

					if ((token+1)->getKind() == Token::REG && (token+2)->getKind() == Token::COMMA && ((token+3)->getKind() == Token::INT || (token+3)->getKind() == Token::HEXINT) 
							&& (token+4)->getKind() == Token::LPAREN && (token+5)->getKind() == Token::REG && (token+6)->getKind() == Token::RPAREN 
							&& (token+7) == line->end()) {
						
						int64_t reg1 = (token+1)->toNumber();
						int64_t reg2 = (token+5)->toNumber();
						if (reg1 < LOW_R || reg1 > MAX_R || reg2 < LOW_R || reg2 > MAX_R) {
							throw AssemblyFailure("ERROR: Invalid register");
						}

						if ((token+3)->getKind() == Token::INT) {
							int64_t x = (token+3)->toNumber();
                                                	if (x < LOW_D || x > MAX_D) {
								throw AssemblyFailure("ERROR: immediate out of range");
							}
						}
						else if ((token+3)->getKind() == Token::HEXINT) {
							int64_t x = (token+3)->toNumber();
							if (x > MAX_H) {
								throw AssemblyFailure("ERROR: immediate out of range");
							}
						}
						token += 6;
						++line_count;
						continue;
					}
					else {
						throw AssemblyFailure("ERROR");
					}
				}

				else if (txt == "beq" || txt == "bne") {

					for (int i = 1; i <= 5; ++i) {
                                                if ((token+i) == line->end()) {
                                                        throw AssemblyFailure("ERROR");
                                                }
                                        }

					if ((token+1)->getKind() == Token::REG && (token+2)->getKind() == Token::COMMA && (token+3)->getKind() == Token::REG && (token+4)->getKind() == Token::COMMA 
							&& ((token+5)->getKind() == Token::INT || (token+5)->getKind() == Token::HEXINT || (token+5)->getKind() == Token::ID) 
							&& (token+6) == line->end()) {
						
						int64_t reg1 = (token+1)->toNumber();
                                                int64_t reg2 = (token+3)->toNumber();
						if (reg1 < LOW_R || reg1 > MAX_R || reg2 < LOW_R || reg2 > MAX_R) {
                                                        throw AssemblyFailure("ERROR: Invalid register");
                                                }

                                                if ((token+5)->getKind() == Token::INT) {
                                                        int64_t x = (token+5)->toNumber();
                                                        if (x < LOW_D || x > MAX_D) {
                                                                throw AssemblyFailure("ERROR: immediate out of range");
                                                        }
                                                }
                                                else if ((token+5)->getKind() == Token::HEXINT) {
                                                        int64_t x = (token+5)->toNumber();
                                                        if (x > MAX_H) {
                                                                throw AssemblyFailure("ERROR: immediate out of range");
                                                        }
                                                }
                                                token += 5;
                                                ++line_count;
                                                continue;
					}
					else {
						throw AssemblyFailure("ERROR");
					}
				}
			}	
			
			else {
				throw AssemblyFailure("ERROR");
			}
	  	}
  	}
  } catch (AssemblyFailure &f) {
	  std::cerr << f.what() << std::endl;
	  return 1;
  }

  int64_t pc = 0;

  // Pass 2 - Assembly
  try {
	  for (auto &line : program) {
		  for (auto token = line->begin(); token != line->end(); ++token) {

			  Token::Kind kd = token->getKind();
			  std::string txt = token->getLexeme();
			  
			  if (kd == Token::LABEL) {
				  continue;
			  }
			  else if (kd == Token::WORD) {
				  pc += 4;

				  if ((token+1)->getKind() == Token::INT || (token+1)->getKind() == Token::HEXINT) {
					  print_w((token+1)->toNumber());
				  }
				  else {
					  if (labels.count((token+1)->getLexeme())) {
						  print_w(labels[(token+1)->getLexeme()]);
					  }
					  else {
						  throw AssemblyFailure("ERROR: label not found");
					  }
				  }
				  ++token;
				  continue;
			  }
			  else if (r_form.count(txt)) {
				  pc += 4;

				  int64_t func = 0;
				  int64_t d = 0;
				  int64_t s = 0;
				  int64_t t = 0;
				  int64_t op = r_form[txt];
				  
				  if (txt == "add" || txt == "sub" || txt == "slt" || txt == "sltu") {
					  d = (token+1)->toNumber();
					  s = (token+3)->toNumber();
					  t = (token+5)->toNumber();
					  token += 5;
				  }
                                
				  else if (txt == "mult" || txt == "multu" || txt == "div" || txt == "divu") {
					  s = (token+1)->toNumber();
					  t = (token+3)->toNumber();
					  token += 3;
				  }
                                
				  else if (txt == "mfhi" || txt == "mflo" || txt == "lis") {
					  d = (token+1)->toNumber();
					  token += 1;
				  }
				  
				  else if (txt == "jr" || txt == "jalr") {
					  s = (token+1)->toNumber();
					  token += 1;
				  }

				  print_r(func, d, s, t, op);
				  continue;
			  }
			  else if (i_form.count(txt)) {
				  pc += 4;

				  int64_t op = i_form[txt];
				  int64_t s = 0;
				  int64_t t = 0;
				  int64_t im = 0;
				  
				  if (txt == "sw" || txt == "lw") {
					  t = (token+1)->toNumber();
					  s = (token+5)->toNumber();
					  im = (token+3)->toNumber();
					  token += 6;
				  }
				  else if (txt == "beq" || txt == "bne") {
					  s = (token+1)->toNumber();
					  t = (token+3)->toNumber();

					  // check if label operand is valid
					  if ((token+5)->getKind() == Token::ID) {
						  if (!labels.count((token+5)->getLexeme())) {
							  throw AssemblyFailure("ERROR: label not defined");
						  }

						  im = (labels[(token+5)->getLexeme()] - pc) / 4;
						  
						  if (im < LOW_D || im > MAX_D) {
							  throw AssemblyFailure("ERROR");
						  }
					  }
					  else {
						  im = (token+5)->toNumber();
					  }
					  token += 5;
				  }

				  print_i(op, s, t, im);
				  continue;
			  }
		  }
	  }
  } catch (AssemblyFailure &f) {
	  std::cerr << f.what() << std::endl;
	  return 1; 
  }

  // print labels table
  for (auto &label : labels) {
	  std::cerr << label.first << " " << label.second << std::endl;
  }

  return 0;
}
