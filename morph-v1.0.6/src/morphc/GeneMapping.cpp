// Author: Tim Diels <timdiels.m@gmail.com>

#include "GeneMapping.h"
#include <morphc/util.h>
#include <morphc/TabGrammarRules.h>

using namespace std;

namespace MORPHC {

GeneMapping::GeneMapping(string path)
{
	// TODO don't assume input it correct in plain inputs.
	// TODO input validation on all read plain files, allow easier formats. Plain = clean, well-documented format. Bin = fast binary format.

	// Read plain file
	{
	read_file(path, [this](const char* begin, const char* end) {
		using namespace boost::spirit::qi;

		auto on_line = [this](const std::vector<std::string>& line) {
			auto gene_name = line.at(0);
			to_lower(gene_name);
			vector<std::string> mapped_names(line.begin()+1, line.end());
			for (auto& name : mapped_names) {
				to_lower(name);
			}

			auto it = mapping.find(gene_name);
			if (it != mapping.end()) {
				cerr << "Warning: Found mappings on multiple lines for: " << gene_name << "\n";
				auto& genes = it->second;
				genes.insert(genes.end(), mapped_names.begin(), mapped_names.end());
			}
			else {
				mapping.emplace(gene_name, mapped_names);
			}
		};

		TabGrammarRules rules;
		parse(begin, end, rules.line[on_line] % eol);
		return begin;
	});

	// Get rid of duplicates
	for (auto& p : mapping) {
		auto& genes = p.second;
		sort(genes.begin(), genes.end());
		auto unique_end = unique(genes.begin(), genes.end());
		genes.erase(unique_end, genes.end());
	}
	}
}

const std::vector<std::string>& GeneMapping::get(std::string gene) const {
	return mapping.at(gene);
}

bool GeneMapping::has(std::string gene) const {
	return mapping.find(gene) != mapping.end();
}

}
