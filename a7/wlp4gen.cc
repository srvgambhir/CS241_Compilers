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
		else if ((type1 == "int" && type2 == "int") || (type1 == "int*" && type2 == "int*")) {
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
	
	/*
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
