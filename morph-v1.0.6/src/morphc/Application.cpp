// Author: Tim Diels <timdiels.m@gmail.com>

#include "Application.h"
#include <iostream>
#include <iomanip>
#include <boost/filesystem.hpp>
#include "ublas.h"
#include "util.h"
#include "ErrorType.h"

using namespace std;
namespace ublas = boost::numeric::ublas;
using namespace ublas;
namespace fs = boost::filesystem;

namespace MORPHC {

Application::Application(int argc, char** argv)
{
	cout << setprecision(9);

	try {
		ensure(argc == 5 || argc == 6, "Invalid argument count", ErrorType::GENERIC);

		config_path = argv[1];
		job_list_path = argv[2];
		output_path = argv[3];
		istringstream str(argv[4]);
		str >> top_k;
		ensure(!str.fail(), "top_k argument must be an integer", ErrorType::GENERIC);
		ensure(top_k > 0, "top_k must be >0", ErrorType::GENERIC);
		if (argc == 6) {
			ensure(string(argv[5]) == "--output-yaml", "Invalid 5th argument", ErrorType::GENERIC);
			output_yaml = true;
		}
		else {
			output_yaml = false;
		}
	}
	catch (const exception& e) {
		cerr << "USAGE: morphc path/to/config.yaml path/to/joblist.yaml path/to/output_directory top_k [--output-yaml]\n"
			<< "\n"
			<< "top_k = max number of candidate genes to save in outputted rankings\n"
			<< "--output-yaml = when specified, rankings are saved in yaml format, otherwise they are saved in plain text format\n"
			<< "\n"
			<< "RETURN CODES:\n";
		std::array<std::string, 3> error_descriptions = {"No error", "Generic error", "GOI contains gene with invalid name"};
		for (int i=0; i<error_descriptions.size(); i++) {
			cerr << "  " << i << ": " << error_descriptions.at(i) << "\n";
		}
		cerr << "\n" << endl;
		throw;
	}
	// TODO look for asserts and see whether they should perhaps be needed at release as well
}

void Application::run() {
	load_config();
	load_jobs();

	for (auto& species_ : species) {
		species_.run_jobs(output_path, top_k, *cache.get(), output_yaml);
	}
}

void Application::load_config() {
	YAML::Node config = YAML::LoadFile(config_path);
	string data_root = config["species_data_path"].as<string>(".");
	cache = make_unique<Cache>(config["cache_path"].as<string>());

	for (auto species_ : config["species"]) {
		species.emplace_back(data_root, species_);
	}
}

void Application::load_jobs() {
	YAML::Node job_list = YAML::LoadFile(job_list_path);
	string data_root = job_list["data_path"].as<string>(".");

	for (auto job_group : job_list["jobs"]) {
		string species_name = job_group["species_name"].as<string>();
		auto matches_name = [&species_name](const Species& s){
			return s.get_name() == species_name;
		};
		auto it = find_if(species.begin(), species.end(), matches_name);
		if (it == species.end()) {
			throw runtime_error("Unknown species in job list: " + species_name);
		}

		string data_root2 = prepend_path(data_root, job_group["data_path"].as<string>("."));

		for (auto goi : job_group["genes_of_interest"]) {
			it->add_job(data_root2, goi);
		}
	}
}

}

