#include <vector>
#include <regex>
#include <iostream>
#include <string>
#include <list>
#include <functional>
#include <unordered_map>
#include <numeric>

using namespace std;

regex token_pattern(R"((\()|(\))|([a-zA-Z_]\w*)|((?:-?\d+)(?:\.\d+)?)|(\+|\-|\*|\/))",std::regex::ECMAScript | std::regex::icase | std::regex::optimize);

enum token_type { none = 0, left_parenthese = 1, right_parenthese, name, number, op };

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
unordered_map <string, double> constants = { {"pi", 3.14159265358979323846}, {"e", 2.71828182845904523536} };

void err(const string& msg) {
	cout << msg << endl;
}

void build_ast(list<token>& tokens, list<token>::iterator& cur_it, node& parent_node) {
	node new_node;
	for (; cur_it != tokens.end(); ++cur_it) {
		switch (cur_it->t)
		{
		case left_parenthese:
			++cur_it;
			if (cur_it == tokens.end()) {
				err("'(' unbanlanced.");
				throw exception();
			}
			if (cur_it->t != op) {
				err("an operator required after '('.");
				throw exception();
			}
			build_ast(tokens, cur_it, new_node);
			parent_node.children.push_back(new_node);
			break;
		case right_parenthese: 
			return;
			break;
		case op:
			parent_node.tok = *cur_it;
			break;
		default:
			parent_node.children.push_back(*cur_it);
			break;
		}
	}
}

double eval(node & cur) {
	switch (cur.tok.t) {
	case op:
		return env[cur.tok.s](cur.children);
	case number:
		return stod(cur.tok.s);
	case name:
		return constants[cur.tok.s];
	default:
		return 0.0;
	}
}

int main() {

	env["+"] = [](vector<node>& vec)->double {
		if (!vec.size()) return 0.0;
		double res = 0.0;
		for (auto& n : vec)
			res += eval(n);
		return res;
	};
	env["-"] = [](vector<node>& vec)->double {
		if (!vec.size()) return 0.0;
		if (vec.size() == 1) return -eval(vec[0]);
		double res = eval(vec[0]);
		auto it = vec.begin();
		++it;
		for (; it != vec.end(); ++it)
			res -= eval(*it);
		return res;
	};
	env["*"] = [](vector<node>& vec)->double {
		if (!vec.size()) return 0.0; 
		double res = 1.0;
		for (auto& n : vec)
			res *= eval(n);
		return res;
	};
	env["/"] = [](vector<node>& vec)->double {
		if (!vec.size()) return 0.0;
		double res = eval(vec[0]);
		if (vec.size() == 1) return 1.0/res;
		auto it = vec.begin();
		++it;
		for (; it != vec.end(); ++it)
			res /= eval(*it);
		return res; 
	};

	cout.precision(9);

	for (;;) {
	
		string expr;
		smatch match;
		list<token> tokens;

		cout << "TS >";
		getline(std::cin, expr);
		
		while (std::regex_search(expr, match, token_pattern)) {
			auto find_type = [&match]() -> unsigned {
				for (unsigned index = 1; index < match.size(); ++index)
					if (match[index].matched) return index;
				return 0;
			};

			auto type = find_type();
			switch (type) {
			case 0: throw exception(""); break;
			case 1: case 2:
			case 3: case 4:
			case 5: tokens.push_back({ string(match[type].first, match[type].second),(token_type)type }); break;
			}
			expr = match.suffix();
		}

		ast.children.clear();
		ast.tok.t = none;
		auto beg = tokens.begin();
		build_ast(tokens, beg, ast);

		cout << eval(ast.children[0]) << endl;
	}
}