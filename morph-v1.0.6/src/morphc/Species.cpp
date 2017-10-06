// Author: Tim Diels <timdiels.m@gmail.com>

#include "Species.h"
#include <boost/regex.hpp>
#include <morphc/GeneExpression.h>
#include <morphc/Clustering.h>
#include <morphc/GeneMapping.h>
#include <morphc/util.h>
#include <morphc/GOIResult.h>
#include "GenesOfInterest.h"
#include <iomanip>

using namespace std;

namespace MORPHC {

Species::Species(string parent_data_root, const YAML::Node node)
:	species(node)
{
	get_name();
	data_root = prepend_path(parent_data_root, node["data_path"].as<string>("."));
}

void Species::add_job(std::string data_root, const YAML::Node& node) { // TODO load later instead of right away
	gois.emplace_back(data_root, node);
}

void Species::run_jobs(string output_path, int top_k, Cache& cache, bool output_yaml) {
	if (gois.empty())
		return;

	// Load gene mapping
	unique_ptr<GeneMapping> gene_mapping;
	if (species["gene_mapping"]) {
		gene_mapping = make_unique<GeneMapping>(prepend_path(data_root, species["gene_mapping"].as<string>()));
	}

	// Regex for gene name validation
	boost::regex gene_pattern(species["gene_pattern"].as<string>(), boost::regex::perl|boost::regex::icase);

	// Load gois
	std::vector<GenesOfInterest> gois;
	for (auto& p : this->gois) {
		gois.emplace_back(p.first, p.second, gene_pattern);
		auto& goi = gois.back();
		if (gene_mapping.get()) {
			goi.apply_mapping(*gene_mapping);
		}
		auto size = goi.get_genes().size();
		if (size < 5) {
			cout << "Dropping GOI " << goi.get_name() << ": too few genes: " << size << " < 5\n";
			gois.pop_back();
		}
	}

	// For each GeneExpression, ge.Cluster, GOI calculate the ranking and keep the best one per GOI
	map<int, GOIResult> results; // goi index -> result for goi
	for (auto gene_expression_description : species["expression_matrices"]) {
		auto gene_expression = make_shared<GeneExpression>(data_root, gene_expression_description, cache);

		// translate gene names to indices; and drop genes missing from the gene expression data
		std::vector<std::vector<size_type>> gois_indices; // gois, but specified by gene indices, not names
		for (int i=0; i < gois.size(); i++) {
			gois_indices.emplace_back();
			for (auto gene : gois.at(i).get_genes()) {
				if (gene_expression->has_gene(gene)) {
					gois_indices.back().emplace_back(gene_expression->get_gene_index(gene));
				}
			}
		}

		// generate correlations
		{
			// distinct union all left over genes of interest
			std::vector<size_type> all_genes_of_interest;
			for (auto& goi : gois_indices) {
				all_genes_of_interest.insert(all_genes_of_interest.end(), goi.begin(), goi.end());
			}
			sort(all_genes_of_interest.begin(), all_genes_of_interest.end());
			auto duplicate_begin = unique(all_genes_of_interest.begin(), all_genes_of_interest.end());
			all_genes_of_interest.erase(duplicate_begin, all_genes_of_interest.end());

			// generate the correlations we need
			gene_expression->generate_gene_correlations(all_genes_of_interest);
			gene_expression->dispose_expression_data();
		}

		// clustering
		for (auto clustering_ : gene_expression_description["clusterings"]) {
			auto clustering = make_shared<Clustering>(gene_expression, data_root, clustering_, cache);
			int goi_index=0;
			for (int i=0; i < gois_indices.size(); i++) {
				auto& goi = gois_indices.at(i);
				cout << get_name() << ", " << gois.at(i).get_name() << ", " << gene_expression->get_name() << ", " << clustering->get_name();
				cout.flush();
				if (goi.size() < 5) {
					cout << ": Skipping: Too few genes of interest found in dataset: " << goi.size() << " < 5\n";
					continue;
				}
				else {
					// Rank genes
					string name = (make_string() << get_name() << "__" << gois.at(i).get_name() << ".txt").str();
					replace(begin(name), end(name), ' ', '_');
					auto ranking = make_unique<Ranking>(goi, clustering, name);
					cout << ": AUSR=" << setprecision(2) << fixed << ranking->get_ausr() << "\n";

					auto result_it = results.find(i);
					if (result_it == results.end()) {
						result_it = results.emplace(piecewise_construct, make_tuple(i), make_tuple()).first;
					}

					auto& result = result_it->second;
					result.add_ausr(ranking->get_ausr());
					if (!result.best_ranking.get() || *ranking > *result.best_ranking) {
						result.best_ranking = std::move(ranking);
					}
				}
				goi_index++;
			}
		}

		gene_expression->dispose_correlations();
	}

	GeneDescriptions gene_descriptions(prepend_path(data_root, species["gene_descriptions"].as<string>()));
	for (auto& p : results) {
		auto& result = p.second;
		result.best_ranking->save(output_path, top_k, gene_descriptions, species["gene_web_page"].as<string>(), gois.at(p.first), result.get_average_ausr(), output_yaml);
	}
}

string Species::get_name() const {
	return species["name"].as<string>();
}

}
