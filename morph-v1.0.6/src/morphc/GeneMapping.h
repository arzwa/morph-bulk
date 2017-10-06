// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <boost/noncopyable.hpp>

namespace MORPHC {

/**
 * Maps gene names to other gene names (for mapping between different naming schemes)
 *
 * Format of gene mapping file: tab-separated, first column = src gene, each other columns is a dst gene (for src -> dst mapping)
 *
 * Gene names are matched in a case-insensitive manner.
 */
class GeneMapping : public boost::noncopyable {
public:
	/**
	 * Load gene mapping
	 */
	GeneMapping(std::string path);

	/**
	 * Get mapped gene names
	 *
	 * @param gene Lower case gene name
	 * @returns vector with size > 0
	 */
	const std::vector<std::string>& get(std::string gene) const;

	/**
	 * Whether mapping is present
	 *
	 * @param gene Lower case gene name
	 */
	bool has(std::string gene) const;

private:
	std::unordered_map<std::string, std::vector<std::string>> mapping; // ns1:gene -> ns2:gene+
};

}
