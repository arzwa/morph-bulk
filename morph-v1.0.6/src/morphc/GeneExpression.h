// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <string>
#include <map>
#include <morphc/Cache.h>
#include <boost/noncopyable.hpp>
#include <yaml-cpp/yaml.h>
#include "ublas.h"

namespace MORPHC {

typedef matrix GeneCorrelations;

class GeneExpression : public boost::noncopyable
{
public:
	/**
	 * Load gene expression from file
	 */
	GeneExpression(std::string data_root, const YAML::Node, Cache&);

	void generate_gene_correlations(const std::vector<size_type>& all_goi);

	/**
	 * Get correlation matrix of genes (rows) and genes of interest (columns).
	 *
	 * Size of matrix: size(genes), size(genes)).
	 * mat(i,j) = correlation between gene with index i and gene with index j.
	 * mat(i,j) only has a valid value if j refers to a gene of interest.
	 */
	const GeneCorrelations& get_gene_correlations() const; // TODO only fill the correlation columns corresponding to a gene of interest

	/**
	 * Get row index of gene in gene_correlations matrix
	 */
	size_type get_gene_index(std::string name) const;
	std::string get_gene_name(size_type index) const;
	bool has_gene(std::string name) const;

	/**
	 * Get column index of gene in gene_correlations matrix
	 */
	size_type get_gene_column_index(size_type gene_row_index) const;

	std::string get_name() const;

	/**
	 * Get all gene indices, sorted
	 */
	const std::vector<size_type>& get_genes() const;

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version);

	/**
	 * Load gene expression from plain text file
	 */
	void load_plain(std::string path);

	void dispose_expression_data();

	/**
	 * Dispose gene correlations data (frees memory)
	 */
	void dispose_correlations();

private:
	std::string name; // name of dataset

	matrix expression_matrix; // row_major
	GeneCorrelations gene_correlations;

	std::vector<size_type> genes; // sorted list of genes
	std::unordered_map<std::string, size_type> gene_indices; // all genes, name -> row index of gene in gene_correlations
	std::unordered_map<size_type, std::string> gene_names;
	std::unordered_map<size_type, size_type> gene_column_indices; // gene row index -> columng index of gene in gene_correlations
};


/////////////////////
// hpp

template<class Archive>
void GeneExpression::serialize(Archive& ar, const unsigned int version) {
	ar & expression_matrix;
	ar & genes;
	ar & gene_indices;
	ar & gene_names;
	ar & gene_column_indices;
}

}
