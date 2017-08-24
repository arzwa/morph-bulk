// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>
#include <boost/regex.hpp>
#include <morphc/GeneMapping.h>

namespace MORPHC {

class GenesOfInterest
{
public:
	GenesOfInterest(std::string data_root, const YAML::Node& genes_of_interest, const boost::regex& gene_pattern);
	const std::vector<std::string>& get_genes() const;
	std::string get_name() const;

	/**
	 * Apply mappings to genes
	 */
	void apply_mapping(const GeneMapping&);

private:
	std::string name;
	std::vector<std::string> genes;
};

}
