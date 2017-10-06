// Author: Tim Diels <timdiels.m@gmail.com>

#include "StringMapping.h"
#include "util.h"
#include <morphc/TabGrammarRules.h>

using namespace std;

namespace MORPHC {

StringMapping::StringMapping(string path)
{
	read_file(path, [this](const char* begin, const char* end) {
		using namespace boost::spirit::qi;

		auto on_line = [this](const std::vector<std::string>& line) {
			auto gene_name = line.at(0);
			to_lower(gene_name);
			auto description = line.at(1);
			if (!mapping.emplace(gene_name, description).second) {
				cout << "Warning: Found multiple mappings for: " << gene_name << "\n";
			}
		};

		TabGrammarRules rules;
		parse(begin, end, rules.line[on_line] % eol);
		return begin;
	});
}

const std::unordered_map<std::string, std::string>& StringMapping::get() const {
	return mapping;
}

}
