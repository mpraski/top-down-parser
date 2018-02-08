//============================================================================
// Name        : ll_1.cpp
// Author      : Marcin Praski
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <initializer_list>

enum class SymbolType { TERM, NON_TERM };

const std::string STR_TERM = "T";
const std::string STR_NON_TERM = "N";

class Symbol {
private:
	SymbolType type;
	std::string value;
public:
	Symbol(): type(SymbolType::TERM), value("e") {}
	Symbol(SymbolType type, std::string value): type(type), value(value) {}

	const std::string Value() const
	{
		return value;
	}

	bool IsTerminal() const
	{
		return type == SymbolType::TERM;
	}

	bool operator==(const Symbol &other) const
	{
		return (type == other.type && value == other.value);
	}

	bool operator!=(const Symbol &other) const
	{
		return (type != other.type || value != other.value);
	}

	friend std::ostream & operator<<(std::ostream & _stream, Symbol const & s) {
		_stream << (s.IsTerminal() ? STR_TERM : STR_NON_TERM) << "(`" << s.Value() << "`)";
		return _stream;
	}
};

namespace std {
template <> struct hash<Symbol> {
    std::size_t operator()(const Symbol& s) const
    {
      using std::size_t;
      using std::hash;

      return hash<std::string>()(s.Value()) ^ hash<bool>()(s.IsTerminal());
    }
  };
}

using Production = std::vector<Symbol>;
using SymbolSet = std::unordered_set<Symbol>;
using ParsingTable = std::unordered_map<Symbol, std::unordered_map<Symbol, Production>>;

const Symbol EPS(SymbolType::TERM, "e");
const Symbol END(SymbolType::NON_TERM, "end");

class Grammar {
private:
	std::unordered_map<Symbol, std::vector<Production>> grammar;
	std::unordered_set<Symbol> terminals;
	Symbol first;
public:
	Grammar(std::initializer_list<std::pair<Symbol, std::vector<Production>>> l)
	{
		if(l.size() > 0)
		{
			first = l.begin()->first;
		}

		for (const auto& it : l)
		{
			grammar[it.first] = it.second;

			for(const auto& p : it.second)
			{
				for(const auto& s : p)
				{
					if(s.IsTerminal()) terminals.insert(s);
				}
			}
		}

		terminals.insert(END);
	}

	using Iter = std::unordered_map<Symbol, std::vector<Production>>::const_iterator;

	const Iter begin() const
	{
		return grammar.cbegin();
	}

	const Iter end() const
	{
		return grammar.cend();
	}

	const Symbol& First() const
	{
		return first;
	}

	const std::vector<Production>& At(const Symbol& s) const
	{
		return grammar.at(s);
	}

	const std::unordered_set<Symbol>& Terminals() const
	{
		return terminals;
	}
};

void _first(
	const Symbol& s,
	const Grammar& grammar,
	std::unordered_map<Symbol, SymbolSet>& result)
{
	if(!s.IsTerminal())
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
	std::unordered_map<Symbol, SymbolSet>& result)
{
	for(const auto& rule : grammar)
	{
		for(const auto& production : rule.second)
		{
			for(const auto& sym : production)
			{
				if(sym.IsTerminal()) result[sym].insert(sym);
			}
		}
	}

	for(const auto& rule : grammar)
	{
		_first(rule.first, grammar, result);
	}
}

void _follow(
	const Symbol& s,
	const Symbol& prev,
	const Grammar& grammar,
	const std::unordered_map<Symbol, SymbolSet>& first,
	std::unordered_map<Symbol, SymbolSet>& result)
{
	for(const auto& kv : grammar)
	{
		for(const auto& production : kv.second)
		{
			bool found { false };
			for(const auto& sym : production)
			{
				if(!found && sym != s) continue;
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
	const std::unordered_map<Symbol, SymbolSet>& first,
	std::unordered_map<Symbol, SymbolSet>& result)
{
	result[grammar.First()].insert(END);

	for(const auto& rule : grammar)
	{
		_follow(rule.first, END, grammar, first, result);
	}
}

void table(
	const Grammar& grammar,
	const std::unordered_map<Symbol, SymbolSet>& first,
	const std::unordered_map<Symbol, SymbolSet>& follow,
	ParsingTable& result
) {

	auto terminals { grammar.Terminals() };

	for(const auto& set : first)
	{

	}

	for(const auto& rule : grammar)
	{
		for(const auto& production : rule.second)
		{

		}
	}
}

int main() {
	Symbol const E(SymbolType::NON_TERM, "E");
	Symbol const T(SymbolType::NON_TERM, "T");
	Symbol const X(SymbolType::NON_TERM, "X");
	Symbol const Y(SymbolType::NON_TERM, "Y");

	Symbol const LP(SymbolType::TERM, "(");
	Symbol const RP(SymbolType::TERM, ")");
	Symbol const PLUS(SymbolType::TERM, "+");
	Symbol const TIMES(SymbolType::TERM, "*");
	Symbol const INT(SymbolType::TERM, "int");

	Grammar const g = {
			{ E, { {T, X}                } },
			{ X, { {PLUS, E},   {EPS}    } },
			{ T, { {LP, E, RP}, {INT, Y} } },
			{ Y, { {TIMES, T},  {EPS}    } },
	};

	std::unordered_map<Symbol, SymbolSet> first_set;

	first(g, first_set);

	for(const auto& it : first_set)
	{
		if(it.first.IsTerminal()) continue;

		std::cout << "First(" << it.first << "): ";
		for(auto it2 : it.second)
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

		std::cout << "Follow(" << it.first << "): ";
		for(auto it2 : it.second)
		{
			std::cout << it2 << " ";
		}
		std::cout << std::endl;
	}

	return 0;
}
