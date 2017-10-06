// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <boost/spirit/include/qi.hpp>

namespace MORPHC {

/**
 * Grammar rules for tab separated files
 */
template <class Iterator>
class BasicTabGrammarRules {
public:
	BasicTabGrammarRules()
	{
		using namespace boost::spirit::qi;
		using namespace boost::fusion;

		separator = lit("\t");
		field %= as_string[lexeme[*(char_- (separator | eol))]];
		line %= field % separator;

		separator.name("field separator");
		field.name("field");
		line.name("line");
	}

	boost::spirit::qi::rule<Iterator> separator; // field separator
	boost::spirit::qi::rule<Iterator, std::string()> field;
	boost::spirit::qi::rule<Iterator, std::vector<std::string>()> line; // = fields
};

typedef BasicTabGrammarRules<const char*> TabGrammarRules;

}
