#include<iostream>
#include<string>
#include<algorithm>
#include<cmath>
#include<cctype>
#include<vector>
#include<deque>
#include<map>
//using namespace std;
using std::string;
using std::cout;
using std::cin;
using std::to_string;
using std::any_of;
using std::isalpha;
using std::min_element;

//Globals
char OPS[] = { '^','/','*','+','-' };		// Used in input validation.
char* ops_list = OPS;						////

char firstOPS[] = { '^' };								////
char secondOPS[] = { '/', '*' };						// 
char thirdOPS[] = { '+', '-' };							// Used in ordering operations.
														//
char* op_array[3] = { firstOPS,secondOPS,thirdOPS };	////

// Prototypes
int Validate_String(string mathStatement); // Ensure user input is a valid mathematical statement
string Compute_Segment(string mathStatement); // recursively calculate segments defined by parentheses
double Calculate(char op, double x, double y); // calculate x op y.
std::map<int, int, std::greater<int>> Map_Parenthesis(string mathStatement); // Find parenthesis pairings.
std::deque<std::pair<int, int>> Order_Paren_Pairs(std::map<int, int, std::greater<int>> parenMap); // Order parenthesis pairs

int main() {
	while (true) {
		//Input
		string mathStatement;
		cout << "Input:\n";
		std::getline(std::cin, mathStatement);
		if (mathStatement == "quit") {
			break;
		}

		//Remove spaces
		string::iterator end_pos = remove(mathStatement.begin(), mathStatement.end(), ' ');
		mathStatement.erase(end_pos, mathStatement.end());

		int flag_error = Validate_String(mathStatement);

		switch (flag_error) {
		case 1:
			cout << "PARSE ERROR: Please omit alphabetic characters from statement.\n";
			break;
		case 2:
			cout << "PARSE ERROR: Statement must begin and end with a numeric value, not an operator. \n";
			break;
		case 3:
			cout << "PARSE ERROR: A number contained too many decimal points.\n";
			break;
		case 4:
			cout << "PARSE ERROR: Please only input numeric values and operators. Acceptable operators are: ^ * \\ + - \n";
			break;
		case 5:
			cout << "PARSE ERROR: No operators found. Acceptable operators are: ^ * \\ + - \n";
			break;
		}
		if (flag_error) continue;

		//Manage parenthesis
		std::map<int, int, std::greater<int>> parenMap = Map_Parenthesis(mathStatement);
		bool flag_skipMap = false;
		if (parenMap.find(-1) != parenMap.end()) {
			int err = parenMap.find(-1)->second;
			if (err == 1) {
				flag_skipMap = true;
			}
			if (err == 2) {
				cout << "Mismatched parenthesis found, please check input.\n";
				continue;
			}
		}
		if (!flag_skipMap) {
			std::deque<std::pair<int, int>>calc_order = Order_Paren_Pairs(parenMap);
			for (auto& pairing : calc_order) {
				cout << '(' << pairing.second << ',' << pairing.first << ")\n";
			}
			while (!calc_order.empty()) {
				std::pair<int, int> curr_loc = calc_order.front();
				calc_order.pop_front();
				string originalStatement = mathStatement.substr(curr_loc.first + 1, curr_loc.second - curr_loc.first - 1);
				string replacementStatement = Compute_Segment(originalStatement);
				int diff = (originalStatement.length() - replacementStatement.length());	// unsigned int to int type for abs()
				diff = std::abs(diff) + 2;
				for (auto& next_pair : calc_order) {
					if (next_pair.first > curr_loc.first) {
						next_pair.first -= diff;
						next_pair.second -= diff;
					}
				}
				mathStatement.replace(curr_loc.first, originalStatement.length() + 2, replacementStatement);
				cout << mathStatement << "\n";
			}
		}

		string res = Compute_Segment(mathStatement);
		cout << res << "\n";
	}
}

// Functions
int Validate_String(string mathStatement) {

	int flag_error = 0;

	// Prevent alphabetic characters in input.
	if (std::any_of(std::begin(mathStatement), std::end(mathStatement), ::isalpha))
	{
		return flag_error = 1;
	}


	// Isolate numeric values
	size_t opLoc = string::npos;
	std::vector<int> num_bounds;
	do {
		opLoc = mathStatement.find_first_of(ops_list, opLoc + 1);
		if (opLoc == 0 || opLoc == mathStatement.length() - 1) return flag_error = 2;	// error flag 2: statement not well-defined.

		if (opLoc != string::npos) num_bounds.push_back(opLoc + 1);

	} while (opLoc != string::npos);

	if (num_bounds.empty()) return flag_error = 5;	// error flag 5: no operators.

	// test numeric values
	for (int i = -1; i != num_bounds.size(); i++) {
		int numLength = num_bounds.at(0) - 0 - 1;
		int startPoint = 0;
		if (i + 1 == num_bounds.size()) {
			startPoint = num_bounds.at(i);
			numLength = string::npos;
		}
		else if (i != -1) {
			startPoint = num_bounds.at(i);
			numLength = num_bounds.at(i + 1) - num_bounds.at(i) - 1;
		}

		string testStatement = mathStatement.substr(startPoint, numLength);
		int decimalLimit = 0;
		for (auto& character : testStatement) {
			if (character == '.') {
				if (++decimalLimit > 1) return flag_error = 3;	// error flag 3: too many decimal points.
				continue;
			}
			if (!isdigit(character) && (character != '(' && character != ')')) return flag_error = 4;		// error flag 4: not numeric characters.
		}
	}
	return flag_error;
}
string Compute_Segment(string mathStatement) {
	double res = 0.0;
	char op = '|';
	string temp_x = "";

	while (mathStatement.length() != 0) {
		string temp_y;
		size_t opLoc;
		size_t next_opLoc = string::npos;
		size_t prev_opLoc = 0;
		unsigned int i = 0;
		do {
			opLoc = mathStatement.find_first_of(op_array[i++]);
		} while (opLoc == string::npos && i < 3);

		for (i = 0; i < sizeof(OPS); i++) {
			size_t test_next = mathStatement.find(*(ops_list + i), opLoc + 1);
			if (test_next < next_opLoc) next_opLoc = test_next;
		}
		for (i = 0; i < sizeof(OPS); i++) {
			size_t test_prev = mathStatement.rfind(*(ops_list + i), opLoc - 1);
			if (test_prev != string::npos && test_prev > prev_opLoc) prev_opLoc = test_prev + 1;
		}

		if (next_opLoc != string::npos || prev_opLoc != 0) {
			size_t startPoint = 0;
			if (prev_opLoc != string::npos) startPoint = prev_opLoc;
			string originalStatement = mathStatement.substr(startPoint, next_opLoc - startPoint);
			string replacementStatement = Compute_Segment(originalStatement);
			mathStatement.replace(startPoint, originalStatement.length(), replacementStatement);
		}
		else {
			if (opLoc != string::npos) {
				op = mathStatement[opLoc];
				temp_y = mathStatement.substr(0, opLoc);
			}
			else {
				temp_y = mathStatement;
			}

			if (temp_x == "") {
				temp_x = temp_y;
			}
			else {
				double x = stod(temp_x);
				double y = stod(temp_y);
				res += Calculate(op, x, y);
			}
			if (opLoc == string::npos) opLoc--;
			mathStatement.erase(0, ++opLoc);
		}
	}
	return to_string(res);
}
double Calculate(char op, double x, double y) {
	double res = 0.0;
	switch (op) {
	case '+':
		res = x + y;
		break;
	case '-':
		res = x - y;
		break;
	case '*':
		res = x * y;
		break;
	case '/':
		res = x / y;
		break;
	case '^':
		res = pow(x, y);
		break;
	}
	return res;
}
std::map<int, int, std::greater<int>> Map_Parenthesis(string mathStatement) {

	//Error code 1: Missing Parenthesis.
	if (mathStatement.find('(') == string::npos) {
		std::map<int, int, std::greater<int>> error_map;
		error_map[-1] = 1;
		return error_map;
	}

	size_t open_loc = 0, close_loc = 0;
	std::vector<int> opened, closed;
	//find parenthesis occurances
	do {
		open_loc = mathStatement.find('(', open_loc);
		if (open_loc != string::npos) opened.push_back(open_loc++);
		close_loc = mathStatement.find(')', close_loc);
		if (close_loc != string::npos) closed.push_back(close_loc++);
	} while (open_loc != string::npos);

	bool flag_error = false;
	for (int i = 0; i < int(opened.size()); i++) {
		if (opened.at(i) > closed.at(i)) {
			flag_error = true;
			break;
		}
	}
	//Error code 2: Unmatched Parenthesis.
	if (opened.size() != closed.size() || flag_error) {
		std::map<int, int, std::greater<int>> error_map;
		error_map[-1] = 2;
		return error_map;
	}


	std::map<int, std::vector<int>, std::greater<int>> potential_map;
	std::vector<int>::const_iterator i;
	std::vector<int>::const_iterator j;
	// Map potentially paired parenthesis
	for (i = opened.begin(); i != opened.end(); ++i) {
		std::vector<int> temp_closed_vec;
		for (j = closed.begin(); j != closed.end(); ++j) {
			if (*i < *j) {
				temp_closed_vec.push_back(*j);
			}
		}
		potential_map.insert(std::pair <int, std::vector<int>>(*i, temp_closed_vec));
	}

	std::map<int, int, std::greater<int>> paren_map;
	//	Find actual matchings
	while (potential_map.size() > 0) {
		int key = potential_map.begin()->first;
		std::vector<int> temp_vec = potential_map.begin()->second;
		int vec_min = *min_element(temp_vec.begin(), temp_vec.end());

		paren_map.insert(std::pair <int, int>(vec_min, key)); // Swap key/value for optimising ordering

		//Remove paired parenthesis
		potential_map.erase(potential_map.begin());
		for (auto& entry : potential_map) {
			auto& repl_vec = entry.second;
			repl_vec.erase(std::remove(repl_vec.begin(), repl_vec.end(), vec_min), repl_vec.end());
		}
	}
	return paren_map;
}
std::deque<std::pair<int, int>> Order_Paren_Pairs(std::map<int, int, std::greater<int>> parenMap) {
	std::deque<std::pair<int, int>> ret;
	std::pair<int, int> container = std::pair<int, int>((*parenMap.begin()).second, (*parenMap.begin()).first); // Swap key/value back for code readibility
	ret.push_front(container);
	parenMap.erase(container.second);
	while (parenMap.size() > 0) {
		std::map<int, int, std::greater<int>>::iterator curr_pair_it = parenMap.begin();
		std::pair<int, int> curr_pair = std::pair<int, int>((*curr_pair_it).second, (*curr_pair_it).first); // Swap key/value back for remaining pairs
		if (container.first < curr_pair.first) {
			ret.push_front(curr_pair);
		}
		else {
			container = curr_pair;
			ret.push_back(container);
		}
		parenMap.erase(curr_pair_it);
	}
	return ret;
}

