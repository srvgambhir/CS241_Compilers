#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <utility>
#include <algorithm>

using namespace std;

int main() {
	vector<string> alphabet;
	vector<string> states;
	vector<string> accepting_states;
	map<pair<string, string>, string> transitions;
	string initial_state;
	string current_state;

	int counter;

	// read in alphabet
	cin >> counter;
	string alphStr;
	for(int i = 1; i <= counter; ++i) {
		cin >> alphStr;
		alphabet.push_back(alphStr);
	}

	// read in states
	cin >> counter;
	string stateStr;
	for (int i = 1; i <= counter; ++i) {
		cin >> stateStr;
		states.push_back(stateStr);
	}

	// read in initial state -> intitialize current to initial
	cin >> initial_state;
	current_state = initial_state;

	// read in accepting states
	cin >> counter;
	string acceptStr;
	for (int i = 1; i <= counter; ++i) {
		cin >> acceptStr;
		accepting_states.push_back(acceptStr);
	}

	// read in transitions
	cin >> counter;
	pair<string, string> transition;
	string result;
	for (int i = 1; i <= counter; ++i) {
		cin >> transition.first;
		cin >> transition.second;
		cin >> result;
		transitions[transition] = result;
	}

	// read in words to determine "true" or "false"
	string lineStr;
	getline(cin, lineStr);
	while(getline(cin, lineStr)) {
		stringstream line{lineStr};
		string inp;
		pair<string, string> tr;
		bool error = false;
		while(line >> inp) {
			tr.first = current_state;
			tr.second = inp;
			if (transitions.count(tr)) {
				current_state = transitions[tr];
			}
			else {
				error = true;
				break;
			}
		}
		if ((find(accepting_states.begin(), accepting_states.end(), current_state) != accepting_states.end()) && error == false) {
			cout << "true" << endl;
		}
		else {
			cout << "false" << endl;
		}

		current_state = initial_state;
	}

}
