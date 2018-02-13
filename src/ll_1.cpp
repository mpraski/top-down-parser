//============================================================================
// Name        : ll_1.cpp
// Author      : Marcin Praski
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <memory>
#include <vector>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <initializer_list>
#include <experimental/any>
#include <stdexcept>

namespace exp = std::experimental;

const std::string STR_TERM = "T";
const std::string STR_NON_TERM = "N";

class Symbol {
private:
	bool        terminal;
	std::string name;
	exp::any    value;

	Symbol(bool terminal, std::string name):
		terminal(terminal),
		name(name) {}

	Symbol(std::string name, exp::any value):
		terminal(true),
		name(name),
		value(value) {}
public:
	Symbol(): terminal(true) {}

	static std::shared_ptr<Symbol> Terminal(std::string name) {
		return std::shared_ptr<Symbol>(new Symbol(true, name));
	}

	static std::shared_ptr<Symbol> Terminal(std::string name, exp::any value) {
		return std::shared_ptr<Symbol>(new Symbol(name, value));
	}

	static std::shared_ptr<Symbol> NonTerminal(std::string name) {
		return std::shared_ptr<Symbol>(new Symbol(false, name));
	}

	static bool equal(const std::shared_ptr<Symbol>& s1, const std::shared_ptr<Symbol>& s2)
	{
		return *s1 == *s2;
	}

	const std::string Name() const
	{
		return name;
	}

	bool IsTerminal() const
	{
		return terminal;
	}

	bool operator==(const Symbol &other) const
	{
		return (terminal == other.terminal && name == other.name);
	}

	bool operator!=(const Symbol &other) const
	{
		return (terminal != other.terminal || name != other.name);
	}

	friend std::ostream & operator<<(std::ostream & _stream, Symbol const & s) {
		_stream << (s.IsTerminal() ? STR_TERM : STR_NON_TERM) << "(`" << s.Name() << "`)";
		return _stream;
	}
};

namespace std {
	template <> struct hash<Symbol> {
		std::size_t operator()(const Symbol& s) const
		{
		  using std::size_t;
		  using std::hash;

		  return hash<std::string>()(s.Name()) ^ hash<bool>()(s.IsTerminal());
		}
	};

	template <> struct hash<Symbol*> {
		std::size_t operator()(const Symbol *s) const
		{
		   using std::size_t;
		   using std::hash;

		   return hash<std::string>()(s->Name()) ^ hash<bool>()(s->IsTerminal());
		}
	};
}

using SymRef	   = std::shared_ptr<Symbol>;
using Production   = std::vector<SymRef>;
using SymbolSet    = std::unordered_set<SymRef>;
using ParsingTable = std::unordered_map<SymRef, std::unordered_map<SymRef, Production>>;

const auto EPS = Symbol::Terminal("\u03B5");
const auto END = Symbol::Terminal("\u2190");

const Production EPS_PRODUCTION { EPS };

class Grammar {
private:
	std::unordered_map<SymRef, std::vector<Production>> grammar;
	std::unordered_set<SymRef> terminals;
	SymRef first;
public:
	Grammar(std::initializer_list<std::pair<SymRef, std::vector<Production>>> l)
	{
		if(l.size() == 0)
		{
			throw std::invalid_argument("Size of grammar is zero");
		}

		first = l.begin()->first;

		for (const auto& it : l)
		{
			grammar[it.first] = it.second;

			for(const auto& p : it.second)
			{
				for(const auto& s : p)
				{
					if(s->IsTerminal()) terminals.insert(s);
				}
			}
		}

		terminals.insert(END);
	}

	using Iter = std::unordered_map<SymRef, std::vector<Production>>::const_iterator;

	const Iter begin() const
	{
		return grammar.cbegin();
	}

	const Iter end() const
	{
		return grammar.cend();
	}

	const SymRef& First() const
	{
		return first;
	}

	const std::vector<Production>& At(const SymRef& s) const
	{
		return grammar.at(s);
	}

	const std::unordered_set<SymRef>& Terminals() const
	{
		return terminals;
	}
};

void _first(
	const SymRef& s,
	const Grammar& grammar,
	std::unordered_map<SymRef, SymbolSet>& result)
{
	if(!s->IsTerminal())
	{
		for(const auto& production : grammar.At(s))
		{
			for(const auto& sym : production)
			{
				_first(sym, grammar, result);

				result[s].insert(result[sym].begin(), result[sym].end());
				result[s].erase(EPS);

				if(result[sym].find(EPS) == result[sym].end()) goto nope;
			}

			result[s].insert(EPS);

			nope: ;
		}
	}
}

void first(
	const Grammar& grammar,
	std::unordered_map<SymRef, SymbolSet>& result)
{
	for(const auto& sym : grammar.Terminals())
	{
		result[sym].insert(sym);
	}

	for(const auto& rule : grammar)
	{
		_first(rule.first, grammar, result);
	}
}

void _follow(
	const SymRef& s,
	const SymRef& prev,
	const Grammar& grammar,
	const std::unordered_map<SymRef, SymbolSet>& first,
	std::unordered_map<SymRef, SymbolSet>& result)
{
	for(const auto& kv : grammar)
	{
		for(const auto& production : kv.second)
		{
			bool found { false };
			for(const auto& sym : production)
			{
				if(!found && !Symbol::equal(sym, s)) continue;
				else if(!found) {
					found = true;
					continue;
				}

				const SymbolSet set { first.at(sym) };

				result[s].insert(set.cbegin(), set.cend());
				result[s].erase(EPS);

				if(set.find(EPS) == set.end()) goto nope;
			}

			if(found) {
				if(prev != kv.first)
				{
					_follow(kv.first, s, grammar, first, result);
				}

				result[s].insert(result[kv.first].begin(), result[kv.first].end());
			}

			nope: ;
		}
	}
}

void follow(
	const Grammar& grammar,
	const std::unordered_map<SymRef, SymbolSet>& first,
	std::unordered_map<SymRef, SymbolSet>& result)
{
	result[grammar.First()].insert(END);

	for(const auto& rule : grammar)
	{
		_follow(rule.first, END, grammar, first, result);
	}
}

void _table(
	const Grammar& grammar,
	const std::unordered_map<SymRef, SymbolSet>& first,
	const std::unordered_map<SymRef, SymbolSet>& follow,
	ParsingTable& result
) {

	SymbolSet terminals { grammar.Terminals() };

	for(const auto& rule : grammar)
	{
		for(const auto& production : rule.second)
		{
			SymbolSet f { first.at(production[0]) };

			if(!production[0]->IsTerminal() && first.at(production[0]).find(EPS) != f.end())
			{
				for(size_t i = 1; i < production.size(); ++i)
				{
					if(first.at(production[i]).find(EPS) == f.end()) break;
					f.insert(first.at(production[i]).begin(), first.at(production[i]).end());
				}
			}

			if(f.find(EPS) == f.end())
			{
				for(const auto& terminal : terminals)
				{
					if(f.find(terminal) != f.end())
					{
						result[rule.first][terminal] = production;
					}
				}
			} else
			{
				for(const auto& ff : follow.at(rule.first))
				{
					result[rule.first][ff] = EPS_PRODUCTION;
				}
			}
		}
	}
}

void table(
	const Grammar& grammar,
	ParsingTable& p_table
) {
	std::unordered_map<SymRef, SymbolSet> first_set;
	std::unordered_map<SymRef, SymbolSet> follow_set;

	first(grammar, first_set);
	follow(grammar, first_set, follow_set);

	_table(grammar, first_set, follow_set, p_table);
}

void parse(
	const std::vector<SymRef>& symbols,
	ParsingTable& p_table
) {
	std::stack<SymRef> stack;

	auto ptr = symbols.begin();

	stack.push(END);
	stack.push(*ptr);

	std::next(ptr);

	while(!stack.empty())
	{
		auto p = stack.top();
		stack.pop();

		if(p->IsTerminal())
		{
			std::next(ptr);
			if(!Symbol::equal(p, *ptr))
			{
				throw std::runtime_error("Malformed input");
			}
		} else
		{
			if(!p_table[p][*ptr].empty())
			{
				throw std::runtime_error("Malformed input");
			}

			for (auto it = p_table[p][*ptr].rbegin(); it != p_table[p][*ptr].rend(); ++it)
			{
				stack.push(*it);
			}
		}
	}
}

int main() {
	const auto E = Symbol::NonTerminal("E");
	const auto T = Symbol::NonTerminal("T");
	const auto X = Symbol::NonTerminal("X");
	const auto Y = Symbol::NonTerminal("Y");

	const auto LP    = Symbol::Terminal("(");
	const auto RP    = Symbol::Terminal(")");
	const auto PLUS  = Symbol::Terminal("+");
	const auto TIMES = Symbol::Terminal("*");
	const auto INT   = Symbol::Terminal("int");

	Grammar const g = {
			{ E, { {T, X}                } },
			{ X, { {PLUS, E},   {EPS}    } },
			{ T, { {LP, E, RP}, {INT, Y} } },
			{ Y, { {TIMES, T},  {EPS}    } },
	};

	/*std::unordered_map<Symbol, SymbolSet> first_set;

	first(g, first_set);

	for(const auto& it : first_set)
	{
		if(it.first.IsTerminal()) continue;

		std::cout << "First( " << it.first << " ): ";
		for(const auto& it2 : it.second)
		{
			std::cout << it2 << " ";
		}
		std::cout << std::endl;
	}

	std::cout << std::endl;

	std::unordered_map<Symbol, SymbolSet> follow_set;

	follow(g, first_set, follow_set);

	for(const auto& it : follow_set)
	{
		if(it.first.IsTerminal()) continue;

		std::cout << "Follow( " << it.first << " ): ";
		for(const auto& it2 : it.second)
		{
			std::cout << it2 << " ";
		}
		std::cout << std::endl;
	}

	std::cout << std::endl;*/

	ParsingTable p_table;

	table(g, p_table);

	for(const auto& row : p_table)
	{
		std::cout << "Entry for row " << *row.first << ": ";
		for(const auto& s : row.second)
		{
			std::cout << *s.first << " [ ";
			for(const auto& p : s.second)
			{
				std::cout << *p << " ";
			}

			std::cout << "] ";
		}

		std::cout << std::endl;
	}

	return 0;
}
