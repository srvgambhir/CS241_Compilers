#include <utility>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <algorithm>
#include <fstream>
using namespace std;

// class representing each node in the parse tree generated by the "Parser" class
class Node {
	public:
		string token;
		string lexeme;
		vector<Node> children;

		// constructor
		Node(string id) : token{id} {}
};

class Parser {
	vector<string> terminals;
	vector<string> nonTerminals;
	string startSymbol;
	vector<vector<string>> productionRules;
	int numStates;

	// shift actions
	map<pair<int, string>, int> shift;

	// reduce actions
	map<pair<int, string>, int> reduce;

	public:

	void dataRead() {

		ifstream inp ("wlp4.cfg");

		int counter;	
	
		// read and store terminals
		inp >> counter;
		string terminal;
		for (int i = 0; i < counter; ++i) {
			inp >> terminal;
			this->terminals.push_back(terminal);
		}

		// read and store non-terminals
		inp >> counter;
		string nonTerminal;
		for (int i = 0; i < counter; ++i) {
			inp >> nonTerminal;
			this->nonTerminals.push_back(nonTerminal);
		}

		// read and store start symbol
		inp >> this->startSymbol;
		
		// read and store production rules in a vector storing vectors of strings
                // the first element in each rule vector is the LHS
                // every element after stores the terminals/non-terminals in the RHS of the rule (as they appear in the rule)
		inp >> counter;
		string line;
		getline(inp, line); // move to next line
		string symbol;
		for (int i = 0; i < counter; ++i) {
			this->productionRules.push_back(vector<string>()); // push back empty vector -> this will store the current rule
			getline(inp, line);
			stringstream ss{line};
			while (ss >> symbol) {
				this->productionRules[i].push_back(symbol);	
			}
		} 

		// read and store number of states
		inp >> this->numStates;

		// read and store the shift and reduce actions
		inp >> counter;
		pair<int, string> p;
		int move;
		string check;
		for (int i = 0; i < counter; ++i) {
			inp >> p.first; // current state
			inp >> p.second; // terminal/nonTerminal -> look ahead
			inp >> check; // check to see if the action is shift or reduce
			inp >> move; // end state of action
			if (check == "shift") {
				this->shift[p] = move;
			}
			else if (check == "reduce") {
				this->reduce[p] = move;
			}
		}

		inp.close();
	}


	void parse() {
		vector<int> stateStack;
		vector<Node> symStack;
		vector<Node> input;

		// create input vector -> manually store BOF and EOF
		Node b{"BOF"};
		b.lexeme = "BOF";
		input.push_back(b);

		string id;
		string lex;
		while (cin >> id) {
			Node n{id};
			cin >> lex;
			n.lexeme = lex;
			input.push_back(n);
		}

		Node e("EOF");
		e.lexeme = "EOF";
		input.push_back(e);

		stateStack.push_back(0);

		int ruleSize;
		int readCount = 0;

		// use to check the shift and reduce vectors
		pair<int, string> check;

		int inputSize = input.size();

		for (int i = 0; i < inputSize; ++i) {

			// current input symbol
			string symbol = input[i].token;
			
			for (int rule = reduction(stateStack.back(), symbol); rule != -1; rule = reduction(stateStack.back(), symbol)) {
				ruleSize = productionRules[rule].size();
				
				// initializze a new node with symbol on LHS of production rule
				Node parent{productionRules[rule][0]};
				
				// insert nodes from symStack that are to be popped off as children of arent node
				parent.children.insert(parent.children.begin(), symStack.end() - ruleSize + 1, symStack.end());

				for (int i = 0; i < ruleSize - 1; ++i) {
                                        stateStack.pop_back();
					symStack.pop_back();
                                }

				// push new node onto symbol stack
				symStack.push_back(parent);

				check.first = stateStack.back(); 
				check.second = parent.token;
				
				// push next state onto stateStack
				stateStack.push_back(shift[check]);
			}
			
			// push current symbol on to symStack
			symStack.push_back(input[i]);
			
			check.first = stateStack.back();
			check.second = symbol;
			
			// reject or continue parse
			if (shift.count(check) == 0) {
				cerr << "ERROR at " << readCount << endl;
				return;
			}
			else {
				stateStack.push_back(shift[check]);
				readCount += 1;
			}
		}

		// check if reduction on rule 0 is possible
                if (symStack.size() != productionRules[0].size() - 1) {
                        cerr << "ERROR at " << readCount + 1 << endl;
                        return;
                }

		// final reduction on rule 0
		Node parent{productionRules[0][0]};
		parent.children.insert(parent.children.begin(), symStack.begin(), symStack.end());

		// print derivation
		print(&parent);
	}

	// return -1 if no reduction possible
	// else return the rule used
	int reduction(int n, string s) {
		pair<int, string> check (n, s);
		if (reduce.count(check) == 0) {
			return -1;
		}
		else {
			return reduce[check];
		}
	}

	// function to print derivation
	void print(Node *n) {
		vector<string>::iterator it;
		vector<Node>::iterator it2 = n->children.begin();
		for (; it2 != n->children.end(); ++it2) {
			string sym = it2->token;
			it = find (terminals.begin(), terminals.end(), sym);
			if (sym != "BOF" && sym != "EOF" && it == terminals.end()) {
				print(&(*it2));
			}
			else {
				cout << sym << " " << it2->lexeme << endl;
			}
		}
		cout << n->token;
		for (auto &t : n->children) {
			cout << " " << t.token;
		}
		cout << endl;
	}
};


int main() {
	Parser p;
	p.dataRead();
	p.parse();
	return 0;
}
