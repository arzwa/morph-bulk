// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>
#include "Cache.h"
#include "Species.h"

namespace MORPHC {

class Application
{
public:
	Application(int argc, char** argv);
	void run();

	std::string get_cache();

private:
	void load_config();
	void load_jobs();

private:
	std::vector<Species> species; // list of species that need to be mined
	std::string config_path;
	std::string job_list_path;
	std::string output_path;
	std::unique_ptr<Cache> cache;
	int top_k;
	bool output_yaml;
};

}
