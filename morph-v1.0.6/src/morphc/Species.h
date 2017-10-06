// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace MORPHC {

class Cache;

/**
 * A species
 */
class Species {
public:
	Species(std::string data_root, const YAML::Node species);

	void add_job(std::string data_root, const YAML::Node& job);

	/**
	 * Find best ranking for each goi, save best rankings in output_path, save at most top_k genes of each ranking
	 */
	void run_jobs(std::string output_path, int top_k, Cache&, bool output_yaml);

	std::string get_name() const;

private:
	const YAML::Node species;
	std::string data_root;
	std::vector<std::pair<std::string, const YAML::Node>> gois;
};

}
