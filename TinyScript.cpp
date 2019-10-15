#include <vector>
#include <regex>
#include <iostream>
#include <string>
#include <list>
#include <functional>
#include <unordered_map>
#include <numeric>

using namespace std;

regex token_pattern(R"((\()|(\))|([a-zA-Z_]\w*)|((?:-?\d+)(?:\.\d+)?)|(\+|\-|\*|\/|\^)|(\S+))", std::regex::ECMAScript | std::regex::icase | std::regex::optimize);

enum token_type { none = 0, left_parenthese, right_parenthese, name, number, op, wrong };

struct token {
	string s = "";
	token_type t = none;
};

struct node {
	token tok;
	vector<node> children;
	node() {};
	node(token& tk) :tok(tk) {};
};

using func_type = function <double(vector<node>&)>;
using environment = unordered_map <string, func_type>;
environment env;
node ast;
unordered_map <string, double> constants = { {"pi", std::acos(-1)}, {"e", 2.71828182845904523536} };
vector<string> table;

void err(const string& msg) {
	cerr << msg << endl;
	throw exception();
}

void build_ast(list<token>& tokens, list<token>::iterator& cur_it, node& parent_node) {
	node new_node;
	unordered_map <string, double>::iterator constant_it;
	environment::iterator op_it;

	for (; cur_it != tokens.end(); cur_it == tokens.end() ? cur_it : ++cur_it) {
		new_node.children.clear();
		switch (cur_it->t) {
		case left_parenthese:
			++cur_it;
			if (cur_it == tokens.end()) {
				err("'(' unbanlanced.");
			}
			if (cur_it->t != op && cur_it->t != name) {
				err("an operator required after '('.");
			}
			build_ast(tokens, cur_it, new_node);
			parent_node.children.push_back(new_node);
			break;
		case right_parenthese:
			return;
			break;
		case name:
			constant_it = constants.find(cur_it->s);
			if (constant_it != constants.end()) {
				parent_node.children.push_back(*cur_it);
				break;
			}
			// else regard current token as a operator
		case op:
			op_it = env.find(cur_it->s);
			if (op_it != env.end()) {
				parent_node.tok = *cur_it;
				break;
			}
			err("no operator '" + cur_it->s + "'.");
			break;
		default:
			parent_node.children.push_back(*cur_it);
			break;
		}
	}
}

double eval(node& cur) {
	unordered_map <string, double>::iterator constant_it;
	environment::iterator op_it;
	switch (cur.tok.t) {
	case number: return stod(cur.tok.s);
	case name:
		constant_it = constants.find(cur.tok.s);
		if (constant_it != constants.end()) return constant_it->second;
		// else regard it as a operator
	case op:
		op_it = env.find(cur.tok.s);
		if (op_it != env.end()) return op_it->second(cur.children);
		else
			err("no operator '" + cur.tok.s + "'.");
		break;
	case none:
	case wrong:
		err("wrong token.");
	}
	return 0.0;
}

int main() {
	env["+"] = [](vector<node>& vec)->double {
		if (!vec.size()) err("at least one parameter is required.");
		double res = 0.0;
		for (auto& n : vec)
			res += eval(n);
		return res;
	};
	env["-"] = [](vector<node>& vec)->double {
		if (!vec.size()) err("at least one parameter is required.");
		if (vec.size() == 1) return -eval(vec[0]);
		double res = eval(vec[0]);
		auto it = vec.begin();
		++it;
		for (; it != vec.end(); ++it)
			res -= eval(*it);
		return res;
	};
	env["*"] = [](vector<node>& vec)->double {
		if (vec.size() < 2) err("at least two parameter is required.");
		double res = 1.0;
		for (auto& n : vec)
			res *= eval(n);
		return res;
	};
	env["/"] = [](vector<node>& vec)->double {
		if (!vec.size()) err("at least one parameter is required.");
		double res = eval(vec[0]);
		if (vec.size() == 1) return 1.0 / res;
		auto it = vec.begin();
		++it;
		for (; it != vec.end(); ++it)
			res /= eval(*it);
		return res;
	};
	env["ln"] = [](vector<node>& vec)->double {
		if (!vec.size()) err("at least one parameter is required.");
		double res = eval(vec[0]);
		if (vec.size() == 1) return log(res);
		else err("'ln' requires only one parameter.");
		return 0.0;
	};
	env["sin"] = [](vector<node>& vec)->double {
		if (!vec.size()) err("at least one parameter is required.");
		double res = eval(vec[0]);
		if (vec.size() == 1) return sin(res);
		else err("'sin' requires only one parameter.");
		return 0.0;
	};
	env["cos"] = [](vector<node>& vec)->double {
		if (!vec.size()) err("at least one parameter is required.");
		double res = eval(vec[0]);
		if (vec.size() == 1) return cos(res);
		else err("'sin' requires only one parameter.");
		return 0.0;
	};
	env["^"] = [](vector<node>& vec)->double {
		if (!vec.size()) err("at least one parameter is required.");
		if (vec.size() != 2) err("'^' requires exactly two parameters.");
		double base = eval(vec[0]);
		double n = eval(vec[1]);
		return pow(base, n);
	};

	cout.precision(9);

	for (;;) {
		string expr;
		smatch match;
		list<token> tokens;

		cout << "TS >";
		getline(std::cin, expr);
		if (!cin.good()) break;

		try {
			while (std::regex_search(expr, match, token_pattern)) {
				auto find_type = [&match]() -> unsigned {
					for (unsigned index = 1; index < match.size(); ++index)
						if (match[index].matched) return index;
					return 0;
				};
				auto type = find_type();
				switch (type) {
				case 0:
				case 1: case 2:
				case 3: case 4:
				case 5: case 6:
					tokens.push_back({ string(match[type].first, match[type].second),(token_type)type }); break;
				}
				expr = match.suffix();
			}

			ast.children.clear();
			ast.tok.t = none;
			auto beg = tokens.begin();
			build_ast(tokens, beg, ast);

			if (!ast.children.size()) err("bad root token.");
			auto result = eval(ast.children[0]);
			cout << result << endl;
		}
		catch (exception e) {
			cerr << "evaluation terminated." << endl;
		}
	}
}