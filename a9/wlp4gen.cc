#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>
#include <sstream>
#include <map>

using namespace std;

// vector for storing terminals
vector<string> terminals{"BOF", "BECOMES", "COMMA", "ELSE", "EOF", "EQ", "GE", "GT", "ID", "IF", "INT", "LBRACE", "LE", "LPAREN", "LT", "MINUS", "NE", "NUM", "PCT", "PLUS", "PRINTLN", "RBRACE", "RETURN", "RPAREN", "SEMI", "SLASH", "STAR", "WAIN", "WHILE", "AMP", "LBRACK", "RBRACK", "NEW", "DELETE", "NULL"};

// top symbol table
map<string, pair<vector<string>, map<string, string>>> tables;

// current procedure being traversed
string procedure = "";

// class for storing each node in our traversal tree
class Tree {
	public:
		string rule;
		string LHS;
		string lexeme; // only for terminals
		string type;
		vector<unique_ptr<Tree>> children;

		Tree(string rule) : rule{rule} {}
};


// determine if "w" is a terminal or not
bool isTerminal(string w) {
	
	auto it = find (terminals.begin(), terminals.end(), w);

	if (it != terminals.end()) {
		return true;
	}

	return false;
}


// read in data and create traversal tree
std::unique_ptr<Tree> read() {
	
	// initialize node with rule
	string rule;
	getline(cin, rule);
	auto tree = make_unique<Tree>(rule);

	// read in lexeme and children
	istringstream ss{rule};
	string LHS;
	ss >> LHS;
	tree->LHS = LHS;
	if (isTerminal(LHS)) {
		ss >> tree->lexeme;
	}
	else {
		string curr;
		while (ss >> curr) {
			tree->children.push_back(std::move(read()));
		}
	}

	return tree;
}


// build our symbol table and check variable and procedure use
bool build(const Tree *t) {

	// variable to check if error encountered
	bool error{false};

	for (auto &child : t->children) {

		// wain function
		if (child->rule == "main INT WAIN LPAREN dcl COMMA dcl RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE") {
			if (tables.count("wain")) {
				cerr << "ERROR: Multiple wain declarations" << endl;
				error = true;
			}
			else {
				procedure = "wain";
				// check type of arguments, and input into tables map
				for (int i = 0; i <= 2; i += 2) {
					if (child->children[i+3]->children[0]->rule == "type INT STAR") {
						tables[procedure].first.push_back("int*");
					}
					else {
						tables[procedure].first.push_back("int");
					}
				}
			}
		}

		// check on variable usage => if it has been declared or not
		else if (child->rule == "factor ID" || child->rule == "lvalue ID") {
			string lex = child->children[0]->lexeme;
			if (!tables[procedure].second.count(lex)) {
				cerr << "ERROR: Variable not declared" << endl;
				error = true;
			}
		}

		// variable declaration
		else if (child->rule == "dcl type ID") {
			string lex = child->children[1]->lexeme;
			if (tables[procedure].second.count(lex)) {
				cerr << "ERROR: Duplicate variable declarations" << endl;
				error = true;
			}
			else {
				if (child->children[0]->rule == "type INT STAR") {
					tables[procedure].second[lex] = "int*";
				}
				else {
					tables[procedure].second[lex] = "int";
				}
			}
		}

		// new procedure declaration
		else if (child->rule == "procedure INT ID LPAREN params RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE") {
			string lex = child->children[1]->lexeme;
			if (tables.count(lex)) {
				cerr << "ERROR: Duplicate procedure declarations" << endl;
				error = true;
			}
			procedure = lex;

			// initialize new entry in tables map
			tables[lex];
		}

		// determine signatures on any procedures
		else if (child->rule == "paramlist dcl" || child->rule == "paramlist dcl COMMA paramlist") {
			if (child->children[0]->children[0]->rule == "type INT STAR") {
				tables[procedure].first.push_back("int*");
			}
			else {
				tables[procedure].first.push_back("int");
			}
		}

		// check procedure calls
		else if (child->rule == "factor ID LPAREN RPAREN" || child->rule == "factor ID LPAREN arglist RPAREN") {
			string lex = child->children[0]->lexeme;
			if (!tables.count(lex) || tables[procedure].second.count(lex)) {
				cerr << "ERROR: Attempting procedure call on variable or an undefined procedure" << endl;
				error = true;
			}
		}

		// if no error encountered so far, set error to value returned by calling "build" on child
		if (!error) {
			error = build(child.get());
		}
		else { break; }

	}
	
	return error;
}


// global variables to help type check
string argProcedure = "";
int argCount = 0;

// type checking
string typeOf(Tree *t) {

	string type{"well-typed"};

	// NUM has type "int"
	if (t->LHS == "NUM") {
		type = "int";
	}
	// NULL has type "int*"
	else if (t->LHS == "NULL") {
		type = "int*";
	}
	// check on ID => variable
	else if (t->LHS == "ID") {
		type = tables[procedure].second[t->lexeme];
	}
	// parentheses
	else if (t->rule == "lvalue LPAREN lvalue RPAREN" || t->rule == "factor LPAREN expr RPAREN") {
		type = typeOf(t->children[1].get());
	}
	else if (t->rule == "expr term" || t->rule == "term factor" || t->rule == "factor ID" || t->rule == "factor NUM" || t->rule == "lvalue ID" || t->rule == "factor NULL") {
		type = typeOf(t->children[0].get());
	}
	// address
	else if (t->rule == "factor AMP lvalue") {
		type = typeOf(t->children[1].get());
		if (type == "int") {
			type = "int*";
		}
		else {
			if (type != "ERROR") {
				cerr << "ERROR: Addressing a pointer" << endl;
				type = "ERROR";
			}
		}
	}
	// dereferencing
	else if (t->rule == "lvalue STAR factor" || t->rule == "factor STAR factor") {
		type = typeOf(t->children[1].get());
		if (type == "int*") {
			type = "int";
		}
		else {
			if (type != "ERROR") {
				cerr << "ERROR: Dereferencing a non-pointer" << endl;
				type = "ERROR";
			}
		}
	}
	// allocating pointer
	else if (t->rule == "factor NEW INT LBRACK expr RBRACK") {
		type = typeOf(t->children[3].get());
		if (type == "int") {
			type = "int*";
		}
		else {
			if (type != "ERROR") {
				cerr << "ERROR: Allocating memory for non-integer" << endl;
				type = "ERROR";
			}
		}
	}
	// mult/div/mod operators
	else if (t->rule == "term term STAR factor" || t->rule == "term term SLASH factor" || t->rule == "term term PCT factor") {
		string type1 = typeOf(t->children[0].get());
		string type2 = typeOf(t->children[2].get());
		if (type1 == "int" && type2 == "int") {
			type = "int";
		}
		else {
			if (type1 != "ERROR" && type2 != "ERROR") {
				cerr << "ERROR: Only integers allowed for multiplication, division, or modulo" << endl;
			}
			type = "ERROR";
		}
	}
	// addition
	else if (t->rule == "expr expr PLUS term") {
		string type1 = typeOf(t->children[0].get());
		string type2 = typeOf(t->children[2].get());
		if (type1 == "ERROR" || type2 == "ERROR") {
			type = "ERROR";
		}
		else if (type1 == "int*" && type2 == "int*") {
			cerr << "ERROR: Addition on two pointers" << endl;
			type = "ERROR";
		}
		else {
			if (type1 == "int" && type2 == "int") {
				type = "int";
			}
			else {
				type = "int*";
			}
		}
	}
	// subtraction
	else if (t->rule == "expr expr MINUS term") {       
		string type1 = typeOf(t->children[0].get());
		string type2 = typeOf(t->children[2].get());
		if (type1 == "ERROR" || type2 == "ERROR") {
			type = "ERROR";
		}
		else if (type1 == "int" && type2 == "int*") {       
			cerr << "ERROR: Subtracting pointer from integer" << endl;
			type = "ERROR";
		}        
		else {			
			if (type1 == "int*" && type2 == "int") {
				type = "int*";        
			}        
			else {        
				type = "int"; 
			}       
		}        
	}
	// procedure call -> at least 1 argument
	else if (t->rule == "factor ID LPAREN arglist RPAREN") {
		argCount = 0;
		argProcedure = t->children[0]->lexeme;
		if (typeOf(t->children[2].get()) != "ERROR") {
			type = "int";	
		}		
		else {
			type = "ERROR";
		}
	}
	// arg check
	else if (t->rule == "arglist expr") {
		int numArgs = tables[argProcedure].first.size();
		if (argCount == numArgs - 1) {
			string currProcedure = argProcedure;
			int currCount = argCount;
			type = typeOf(t->children[0].get());
			if (type != "ERROR") {
				if (tables[currProcedure].first[currCount] != type) {
					cerr << "ERROR: Argument type in procedure call not valid" << endl;
					type = "ERROR";
				}
			}
		}
		else {
			cerr << "ERROR: Incorrect number of arguments for procedure call" << endl;
			type = "ERROR";
		}
	}
	// arg check
	else if (t->rule == "arglist expr COMMA arglist") {
		int numArgs = tables[argProcedure].first.size();
		if (argCount < numArgs) {
			string currProcedure = argProcedure;
			int currCount = argCount;
			type = typeOf(t->children[0].get());
			if (type != "ERROR") {
				if (tables[currProcedure].first[currCount] == type) {
					argProcedure = currProcedure;
					argCount = currCount;
					argCount += 1;
					type = typeOf(t->children[2].get());
				}
				else {
					cerr << "ERROR: Argument type in procedure call not valid" << endl;
					type = "ERROR";
				}
			}
		}
		else {
			cerr << "ERROR: Incorrect number of arguments for procedure call" << endl;
			type = "ERROR";
		}
	}
	// procedure call -> no args
	else if (t->rule == "factor ID LPAREN RPAREN") {
		if (tables[t->children[0]->lexeme].first.size() != 0) {
			cerr << "ERROR: Sending in arguments to procedure with empty signature" << endl;
			type = "ERROR";
		}
		else {
			type = "int";
		}
	}
	// comparison operators
	else if (t->LHS == "test") {
		string type1 = typeOf(t->children[0].get());
		string type2 = typeOf(t->children[2].get());
		if (type1 == "ERROR" || type2 == "ERROR") {
			type = "ERROR";
		}
		else if (type1 == "int" && type2 == "int") {
			t->type = "int";
			type = "well-typed";
		}
		else if (type1 == "int*" && type2 == "int*") {
			t->type = "int*";
			type = "well-typed";
		}
		else {
			cerr << "ERROR: Comparison invalid" << endl;
			type = "ERROR";
		}
	}
	// println
	else if (t->rule == "statement PRINTLN LPAREN expr RPAREN SEMI") {
		type = typeOf(t->children[2].get());
		if (type != "ERROR") {
			if (type != "int") {
				cerr << "ERROR: Can only print integers" << endl;
				type = "ERROR";
			}
			else {
				type = "well-typed";
			}
		}
	}
	// assignment
	else if (t->rule == "statement lvalue BECOMES expr SEMI") {
		string type1 = typeOf(t->children[0].get());
		string type2 = typeOf(t->children[2].get());
		if (type1 == "ERROR" || type2 == "ERROR") {
			type = "ERROR";	
		}
		else if (type1 == type2) {
			type = "well-typed";
		}
		else {
			cerr << "ERROR: Incorrect assignment type" << endl;
			type = "ERROR";
		}
	}
	// deallocation
	else if (t->rule == "statement DELETE LBRACK RBRACK expr SEMI") {
		type = typeOf(t->children[3].get());
		if (type != "ERROR") {
			if (type == "int*") {
				type = "well-typed";
			}
			else {
				cerr << "ERROR: Deallocating non-pointer" << endl;
				type = "ERROR";
			}
		}
	}
	// int declaration
	else if (t->rule == "dcls dcls dcl BECOMES NUM SEMI") {
		if (t->children[1]->children[0]->rule == "type INT") {
			type = typeOf(t->children[0].get());
		}
		else {
			cerr << "ERROR: Attempting to declare non-integer with integer" << endl;
			type = "ERROR";
		}
	}
	// pointer declaration
	else if (t->rule == "dcls dcls dcl BECOMES NULL SEMI") {
                if (t->children[1]->children[0]->rule == "type INT STAR") {
                        type = typeOf(t->children[0].get());
                }
                else {
                        cerr << "ERROR: Attempting to declare non-pointer with NULL" << endl;
                        type = "ERROR";
                }
        }
	// procedure definition
	else if (t->LHS == "procedure") {
		procedure = t->children[1]->lexeme;
		type = typeOf(t->children[6].get()); // dcls
		if (type != "ERROR") {
			type = typeOf(t->children[7].get()); // statements
			if (type != "ERROR") {
				type = typeOf(t->children[9].get()); // return expr
				if (type == "int") {
					type = "well-typed";
				}
				else if (type != "ERROR") {
					cerr << "ERROR: Returning non-integer from procedure" << endl;
					type = "ERROR";
				}
			}
		}
	}
	// wain definition
	else if (t->LHS == "main") {
		procedure = "wain";
		string dcl2 = t->children[5]->children[0]->rule;
		if (dcl2 == "type INT") {
			type = typeOf(t->children[8].get()); // dcls
			if (type != "ERROR") {
                		type = typeOf(t->children[9].get()); // statements
				if (type != "ERROR") {
                			type = typeOf(t->children[11].get()); // return expr
                			if (type == "int") {
                        			type = "well-typed";
                			}
                			else if (type != "ERROR") {
                        			cerr << "ERROR: Returning non-integer from wain" << endl;
                        			type = "ERROR";
                			}
				}
			}
		}
		else {
			cerr << "ERROR: Second argument in wain not an integer" << endl;
			type = "ERROR";
		}
        }
	else {
		for (auto &child : t->children) {
			type = typeOf(child.get());
			if (type == "ERROR") {
				break;
			}	
		}
	}
	return type;
}


// register symbol table
map<string, map<string, int>> offsets;

// register offset
int offsetCount = 0;

// counter for if loop labels
int ifCount = 0;

// counter for while loop variables
int whileCount = 0;

// counter for delete
int deleteCount = 0;

// counter for local var in a procedure
int local = 0;

// code(a) -> store value corresponding to variable a from memory into register 3
void code (string a) {
	cout << "lw $3, " << offsets[procedure][a] << "($29)" << endl;
}

// push(reg) -> push value stored at reg to memory
void push (int reg) {
	cout << "sw $" << reg << ", -4($30)" << endl;
	cout << "sub $30, $30, $4" << endl;
}

// pop(reg) -> store next value in memory into reg
void pop (int reg) {
	cout << "add $30, $30, $4" << endl;
	cout << "lw $" << reg << ", -4($30)" << endl;
}

// function to parse tree and generate code
void Gen(Tree *t) {
	
	// start -> BOF procedures EOF
	if (t->LHS == "start") {
		Gen(t->children[1].get()); // procedures
	}

	else if (t->rule == "procedures procedure procedures") {
		// Generate procedures first => in order to generate wain at the top of assembly
		Gen(t->children[1].get());
		Gen(t->children[0].get());
	}
	
	else if (t->rule == "procedures main") {
		Gen(t->children[0].get());
	}

	// procedure -> INT ID LPAREN params RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE
	else if (t->LHS == "procedure") {
		procedure = t->children[1]->lexeme;
		local = 0;
		Gen(t->children[3].get()); // params
		cout << "F" << procedure << ":" << endl;
		cout << "sub $29, $30, $4" << endl;
		Gen(t->children[6].get()); // dcls
		Gen(t->children[7].get()); // statements
		Gen(t->children[9].get()); // expr
		cout << "add $30, $29, $4" << endl;
		cout << "jr $31" << endl;
	}

	// offsets for parameters in procedures
	else if (t->rule == "params paramlist") {
		int total = tables[procedure].first.size();
		int params = 1;
		Tree *p = t->children[0].get();
		while (p->rule == "paramlist dcl COMMA paramlist") {
			offsets[procedure][p->children[0]->children[1]->lexeme] = 4*(total-params+1);
			++params;
			p = p->children[2].get();
		}
		offsets[procedure][p->children[0]->children[1]->lexeme] = 4*(total-params+1);
	}
	
	// main -> INT WAIN LPAREN dcl COMMA dcl RPAREN LBRACE dcls statements RETURN expr SEMI RBRACE
	else if (t->LHS == "main") {
		string output;

                // Prologue of wain
                output = ";Prologue\n";
                output += ".import print\n";
		output += ".import init\n";
		output += ".import new\n";
		output += ".import delete\n";
                output += "lis $4\t; $4 will always hold 4\n.word 4\n";
                output += "lis $10\t; $10 holds address for print\n.word print\n";
                output += "lis $11\t; $11 will always hold 1\n.word 1\n";
                output += "sub $29, $30, $4\t; setup frame pointer\n";
                cout << output;

		procedure = "wain";
		for (int i = 1; i <= 2; ++i) {
			cout << "sw $" << i << ", -4($30)" << endl;
			cout << "sub $30, $30, $4" << endl;
		}
		Gen(t->children[3].get()); // first param
		Gen(t->children[5].get()); // second param

		// call init
		cout << ";Prologue init" << endl;
		if (tables[procedure].first[0] == "int") {
			cout << "add $2, $0, $0" << endl;
		}
		push(31);
		cout << "lis $5" << endl;
		cout << ".word init" << endl;
		cout << "jalr $5" << endl;
		pop(31);


		// call rest of wain
		Gen(t->children[8].get()); // dcls
		Gen(t->children[9].get()); // statements
		Gen(t->children[11].get()); // expr


		 // Epilogue of wain
                output = ";Epilogue\n";
                output += "add $30, $29, $4\n";
                output += "jr $31\n";
                cout << output;
	}
	
	// enter variable into offsets map
	else if (t->rule == "dcl type ID") {
		if (procedure == "wain") {
			offsets[procedure][t->children[1]->lexeme] = offsetCount;
			offsetCount -= 4;
		}
		else {
			++local;
			offsets[procedure][t->children[1]->lexeme] = -4*(local-1);
		}
	}
	
	else if (t->rule == "expr term" || t->rule == "term factor") {
		Gen(t->children[0].get());
	}
	
	else if (t->rule == "factor ID") {
		code(t->children[0]->lexeme);
	}

	// dereference
	else if (t->rule == "factor STAR factor") {
		Gen(t->children[1].get());
		cout << "lw $3, 0($3)" << endl;
	}
	
	else if (t->rule == "factor LPAREN expr RPAREN") {
		Gen(t->children[1].get());
	}

	// addressing
	else if (t->rule == "factor AMP lvalue") {
		Tree *lvalue = t->children[1].get();
                while (lvalue->rule == "lvalue LPAREN lvalue RPAREN") {
                        lvalue = lvalue->children[1].get();
                }
		if (lvalue->rule == "lvalue ID") {
			cout << "lis $3" << endl;
			cout << ".word " << offsets[procedure][lvalue->children[0]->lexeme] << endl;
			cout << "add $3, $3, $29" << endl;
		}
		else if (lvalue->rule == "lvalue STAR factor") {
			Gen(lvalue->children[1].get());
		}
	}
	
	// basic operations -> +,-,*,/,% 
	else if (t->rule == "expr expr PLUS term" || t->rule == "expr expr MINUS term" || t->rule == "term term STAR factor" || t->rule == "term term SLASH factor" || t->rule == "term term PCT factor") {
		if (t->rule == "expr expr PLUS term") {
			string type1 = typeOf(t->children[0].get());
			string type2 = typeOf(t->children[2].get());
			if (type1 == "int" && type2 == "int") {
				Gen(t->children[0].get());
				push(3);
				Gen(t->children[2].get());
				pop(5);
				cout << "add $3, $5, $3" << endl;
			}
			else if (type1 == "int*" && type2 == "int") {
				Gen(t->children[0].get());
                                push(3);
                                Gen(t->children[2].get());
				cout << "mult $3, $4" << endl;
				cout << "mflo $3" << endl;
				pop(5);
				cout << "add $3, $5, $3" << endl;
			}
			else if (type1 == "int" && type2 == "int*") {
				Gen(t->children[2].get());
                                push(3);
                                Gen(t->children[0].get());
                                cout << "mult $3, $4" << endl;
                                cout << "mflo $3" << endl;
                                pop(5);
                                cout << "add $3, $5, $3" << endl;
			}
		}
		else if (t->rule == "expr expr MINUS term") {
			string type1 = typeOf(t->children[0].get());
                        string type2 = typeOf(t->children[2].get());
                        if (type1 == "int" && type2 == "int") {
                                Gen(t->children[0].get());
                                push(3);
                                Gen(t->children[2].get());
                                pop(5);
                                cout << "sub $3, $5, $3" << endl;
                        }
                        else if (type1 == "int*" && type2 == "int") {
                                Gen(t->children[0].get());
                                push(3);
                                Gen(t->children[2].get());
                                cout << "mult $3, $4" << endl;
                                cout << "mflo $3" << endl;
                                pop(5);
                                cout << "sub $3, $5, $3" << endl;
                        }
                        else if (type1 == "int*" && type2 == "int*") {
                                Gen(t->children[0].get());
                                push(3);
                                Gen(t->children[2].get());
				pop(5);
				cout << "sub $3, $5, $3" << endl;
				cout << "div $3, $4" << endl;
				cout << "mflo $3" << endl;
                        }
		}
		else {
			Gen(t->children[0].get());
			push(3);
			Gen(t->children[2].get());
			pop(5);
			if (t->rule == "term term STAR factor") {
				cout << "mult $5, $3" << endl;
				cout << "mflo $3" << endl;
			}
			else if (t->rule == "term term SLASH factor") {
				cout << "div $5, $3" << endl;
				cout << "mflo $3" << endl;
			}
			else if (t->rule == "term term PCT factor") {
				cout << "div $5, $3" << endl;
				cout << "mfhi $3" << endl;
			}
		}
	}
	
	else if (t->rule == "factor NUM") {
		cout << "lis $3\n.word " << t->children[0]->lexeme << endl;
	}

	// set value of NULL to 1
	else if (t->rule == "factor NULL") {
		cout << "add $3, $0, $11" << endl;
	}
	
	else if (t->rule == "statements statements statement") {
		Gen(t->children[0].get());
		Gen(t->children[1].get());
	}
	
	// print
	else if (t->rule == "statement PRINTLN LPAREN expr RPAREN SEMI") {
		push(1);
		Gen(t->children[2].get());
		cout << "add $1, $3, $0" << endl;
		push(31);
		cout << "jalr $10" << endl;
		pop(31);
		pop(1);
	}
	
	// int declarations
	else if (t->rule == "dcls dcls dcl BECOMES NUM SEMI") {
		Gen(t->children[0].get());
		cout << "lis $5" << endl;
		cout << ".word " << t->children[3]->lexeme << endl;
		push(5);
		Gen(t->children[1].get());
	}

	// pointer declarations
	else if (t->rule == "dcls dcls dcl BECOMES NULL SEMI") {
		Gen(t->children[0].get());
		cout << "add $5, $0, $11" << endl;
		push(5);
		Gen(t->children[1].get());
	}

	// assignment
	else if (t->rule == "statement lvalue BECOMES expr SEMI") {
		Tree *lvalue = t->children[0].get();
		while (lvalue->rule == "lvalue LPAREN lvalue RPAREN") {
			lvalue = lvalue->children[1].get();
		}
		if (lvalue->rule == "lvalue ID") {
			Gen(t->children[2].get());
			cout << "sw $3, " << offsets[procedure][lvalue->children[0]->lexeme] << "($29)" << endl;
		}
		else if (lvalue->rule == "lvalue STAR factor") {
			Gen(t->children[2].get());
			push(3);
			Gen(lvalue->children[1].get());
			pop(5);
			cout << "sw $5, 0($3)" << endl;
		}
	}

	// comparisons
	else if (t->rule == "test expr LT expr") {
		Gen(t->children[0].get());
		push(3);
		Gen(t->children[2].get());
		pop(5);
		if (t->type == "int") {
			cout << "slt $3, $5, $3" << endl;
		}
		else if (t->type == "int*") {
			cout << "sltu $3, $5, $3" << endl;
		}
	}
	else if (t->rule == "test expr GT expr") {
		Gen(t->children[2].get());
                push(3);
                Gen(t->children[0].get());
                pop(5);
		if (t->type == "int") {
                	cout << "slt $3, $5, $3" << endl;
		}
		else if (t->type == "int*") {
			cout << "sltu $3, $5, $3" << endl;
		}
	}
	else if (t->rule == "test expr NE expr") {
		Gen(t->children[0].get());
		push(3);
		Gen(t->children[2].get());
		pop(5);
		if (t->type == "int") {
                        cout << "slt $6, $3, $5" << endl;
                        cout << "slt $7, $5, $3" << endl;
                }
                else if (t->type == "int*") {
                        cout << "sltu $6, $3, $5" << endl;
                        cout << "sltu $7, $5, $3" << endl;
                }
		cout << "add $3, $6, $7" << endl;
	}
	else if (t->rule == "test expr EQ expr") {
		Gen(t->children[0].get());
                push(3);
                Gen(t->children[2].get());
                pop(5);
                if (t->type == "int") {
			cout << "slt $6, $3, $5" << endl;
                	cout << "slt $7, $5, $3" << endl;
		}
		else if (t->type == "int*") {
			cout << "sltu $6, $3, $5" << endl;
                        cout << "sltu $7, $5, $3" << endl;
		}
                cout << "add $3, $6, $7" << endl;
		cout << "sub $3, $11, $3" << endl;
	}
	else if (t->rule == "test expr LE expr") {
		Gen(t->children[2].get());
                push(3);
                Gen(t->children[0].get());
                pop(5);
		if (t->type == "int") {
			cout << "slt $3, $5, $3" << endl;
		}
		else if (t->type == "int*") {
			cout << "sltu $3, $5, $3" << endl;
		}
		cout << "sub $3, $11, $3" << endl;
	}
	else if (t->rule == "test expr GE expr") {
		Gen(t->children[0].get());
                push(3);
                Gen(t->children[2].get());
                pop(5);
		if (t->type == "int") {
			cout << "slt $3, $5, $3" << endl;
		}
		else if (t->type == "int*") {
			cout << "sltu $3, $5, $3" << endl;
		}
		cout << "sub $3, $11, $3" << endl;
	}
	
	// while statement
	else if (t->rule == "statement WHILE LPAREN test RPAREN LBRACE statements RBRACE") {
		int count = whileCount;
		++whileCount;
		cout << "loop" << count << ":" << endl;
		Gen(t->children[2].get());
		cout << "beq $3, $0, endWhile" << count << endl;
		Gen(t->children[5].get());
		cout << "beq $0, $0, loop" << count << endl;
		cout << "endWhile" << count << ":" << endl;
	}

	// if statement
	else if (t->rule == "statement IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE") {
		int count = ifCount;
		++ifCount;
		Gen(t->children[2].get());
		cout << "beq $3, $0, else" << count << endl;
		Gen(t->children[5].get());
		cout << "beq $0, $0, endif" << count << endl;
		cout << "else" << count << ":" << endl;
		Gen(t->children[9].get());
		cout << "endif" << count << ":" << endl;
	}

	// allocate memory -> call NEW
	else if (t->rule == "factor NEW INT LBRACK expr RBRACK") {
		Gen(t->children[3].get());
		cout << "add $1, $3, $0" << endl;
		push(31);
		cout << "lis $5" << endl;
		cout << ".word new" << endl;
		cout << "jalr $5" << endl;
		pop(31);
		cout << "bne $3, $0, 1" << endl;
		cout << "add $3, $11, $0" << endl;
	}

	// deallocate memory -> call DELETE
	else if (t->rule == "statement DELETE LBRACK RBRACK expr SEMI") {
		int count = deleteCount;
		++deleteCount;
		Gen(t->children[3].get());
		cout << "beq $3, $11, " << "skipDelete" << count << endl;
		cout << "add $1, $3, $0" << endl;
                push(31);
                cout << "lis $5" << endl;
                cout << ".word delete" << endl;
                cout << "jalr $5" << endl;
                pop(31);
                cout << "skipDelete" << count << ":" << endl;
	}

	// procedure call with no arguments
	else if (t->rule == "factor ID LPAREN RPAREN") {
		push(29);
		push(31);
		cout << "lis $5" << endl;
		cout << ".word " << "F" << t->children[0]->lexeme << endl;
		cout << "jalr $5" << endl;
		pop(31);
		pop(29);
	}

	// procedure call with arguments
	else if (t->rule == "factor ID LPAREN arglist RPAREN") {
		push(29);
		push(31);
		Tree *a = t->children[2].get();
		while (a->rule == "arglist expr COMMA arglist") {
			Gen(a->children[0].get());
			push(3);
			a = a->children[2].get();
		}
		Gen(a->children[0].get());
		push(3);
		cout << "lis $5" << endl;
		cout << ".word " << "F" << t->children[0]->lexeme << endl;
		cout << "jalr $5" << endl;
		int params = tables[t->children[0]->lexeme].first.size();
		for (int i = 1; i <= params; ++i) {
			pop(5);
		}
		pop(31);
		pop(29);
	}
}


int main() {
	
	// create parse tree
	unique_ptr<Tree> tree = read();
	
	bool error{false};

	// build symbol table
	error = build(tree.get());

	string typeResult{"well-typed"};

	// check type if successful tables build
	if (!error) {
		typeResult = typeOf(tree.get());
	}

	// generate MIPS code if successful tables build and program well-typed
	if (!error && typeResult != "ERROR") {
		Gen(tree.get());
	}

	/*
	// print offsets
	for (auto &it : offsets) {
		for (auto &it2 : it.second) {
			cout << it2.first << " " << it2.second << endl;
		}
	}
	
	// print signatures and variables if no error encountered
	if (!error && typeResult != "ERROR") {
		for (auto &it : tables) {

			// print procedure name and signature
			cerr << it.first << ":";
			for (auto &it2 : it.second.first) {
				cerr << " " << it2;
			}
			cerr << endl;

			// print variables
			for (auto &it3 : it.second.second) {
				cerr << it3.first << " " << it3.second << endl;
			}
		}
	}
	*/
}
