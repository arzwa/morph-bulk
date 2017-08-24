// Author: Tim Diels <timdiels.m@gmail.com>

#include "Clustering.h"
#include "util.h"
#include <boost/spirit/include/qi.hpp>
#include <boost/function_output_iterator.hpp>
#include <utility>
#include <morphc/TabGrammarRules.h>

using namespace std;

namespace MORPHC {

Clustering::Clustering(shared_ptr<GeneExpression> gene_expression, string data_root, const YAML::Node node, Cache& cache)
:	name(node["name"].as<string>()), gene_expression(gene_expression)
{
	auto path = prepend_path(data_root, node["path"].as<string>());
	cache.load_bin_or_plain(path, path + gene_expression->get_name() + ".bin", *this); // Note: as long as we store indices in the bin file, we must save a bin per gene expression matrix
}

void Clustering::load_plain(std::string path) {
	std::vector<size_type> genes;

	// Load
	read_file(path, [this, &genes](const char* begin, const char* end) {
		using namespace boost::spirit::qi;

		std::unordered_map<std::string, Cluster> clusters;
		size_type genes_missing = 0;

		auto on_cluster_item = [this, &clusters, &genes, &genes_missing](const std::vector<std::string>& line) {
			auto gene_name = line.at(0);
			to_lower(gene_name);
			if (!gene_expression->has_gene(gene_name)) {
				// Not all clusterings are generated from an expression matrix.
				// So a clustering can contain genes that are not present in the expression matrix.
				genes_missing++;
				return;
			}
			auto cluster_id = line.at(1);
			auto it = clusters.find(cluster_id);
			if (it == clusters.end()) {
				it = clusters.emplace(piecewise_construct, make_tuple(cluster_id), make_tuple(cluster_id)).first;
			}
			auto index = gene_expression->get_gene_index(gene_name);
			auto& cluster = it->second;
			ensure(!contains(cluster, index),
					(make_string() << "Clustering adds same gene to cluster twice: gene=" << gene_name <<
							", cluster=" << cluster_id).str(),
					ErrorType::GENERIC);
			cluster.add(index);
			genes.emplace_back(index);
		};

		TabGrammarRules rules;
		parse(begin, end, rules.line[on_cluster_item] % eol);

		// Move clusters' values to this->clusters
		this->clusters.reserve(clusters.size());
		for(auto& p : clusters) {
			this->clusters.emplace_back(std::move(p.second));
		}

		if (genes_missing > 0) {
			cerr << "Warning: " << genes_missing << " genes in clustering not present in expression matrix\n";
		}

		return begin;
	});

	// Group together unclustered genes
	sort(genes.begin(), genes.end());
	clusters.emplace_back(" unclustered"); // the leading space is to avoid accidentally overwriting a cluster in the clustering file named 'unclustered'
	auto& cluster = clusters.back();
	auto& all_genes = gene_expression->get_genes(); // Note: must be ordered
	auto add_to_cluster = boost::make_function_output_iterator([&cluster](const size_type& gene) {
		cluster.add(gene);
	});
	assert(is_sorted(all_genes.begin(), all_genes.end()));
	set_difference(all_genes.begin(), all_genes.end(), genes.begin(), genes.end(), add_to_cluster);
	if (cluster.empty()) {
		clusters.pop_back();
	}
}

Clustering::const_iterator Clustering::begin() const {
	return clusters.begin();
}

Clustering::const_iterator Clustering::end() const {
	return clusters.end();
}

GeneExpression& Clustering::get_source() const {
	return *gene_expression;
}

}
