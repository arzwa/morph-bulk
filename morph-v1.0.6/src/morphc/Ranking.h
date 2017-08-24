// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include "ublas.h"
#include "Clustering.h"
#include "GeneDescriptions.h"
#include <boost/noncopyable.hpp>

namespace MORPHC{

/**
 * Private class of Ranking
 */
class Ranking_ClusterInfo : public boost::noncopyable { // TODO this could be tidier
public:
	Ranking_ClusterInfo(const GeneExpression& gene_expression, const std::vector<size_type>& genes_of_interest, const Cluster& cluster);

	size_type get_goi_count() {
		return goi.size();
	}

	MORPHC::indirect_array goi; // row indices of genes of interest in cluster
	MORPHC::indirect_array goi_columns; // column indices of genes of interest in cluster
	MORPHC::indirect_array candidates; // candidates in cluster (i.e. not goi)
	MORPHC::indirect_array genes; // all genes in cluster

private:
	MORPHC::array goi_;
	MORPHC::array goi_columns_;
};

// TODO refactor
/**
 * Note: A negative ranking value for a gene means it wasn't ranked
 */
class Ranking : public boost::noncopyable // TODO this is not a single ranking, it's a set of rankings (but we've already used the name Rankings/rankings internally)
{
public:

	Ranking(std::vector<size_type> genes_of_interest, std::shared_ptr<Clustering>, std::string name);

	/**
	 * Save top k results in given directory
	 *
	 * Full goi: goi without genes missing in dataset removed
	 */
	void save(std::string directory, int top_k, const GeneDescriptions&, std::string gene_webpage_template, const GenesOfInterest& full_goi, double average_ausr, bool output_yaml);

	double get_ausr() const;
	bool operator>(const Ranking&) const;

private:
	typedef boost::numeric::ublas::vector<double> Rankings;

	const GeneCorrelations& get_gene_correlations();
	const GeneExpression& get_gene_expression();
	void rank_genes(const std::vector<size_type>& genes_of_interest, Rankings& rankings);
	void rank_self(const Rankings& rankings);
	void finalise_ranking(const Rankings& rankings);

	/**
	 * Finalise part of ranking
	 *
	 * Said part is: project(final_rankings, sub_indices)
	 * cluster info must be of the cluster it belongs to
	 *
	 * Note: this func is highly specialised, not very reusable
	 */
	void finalise_sub_ranking(const Rankings& rankings, Rankings& final_rankings, const MORPHC::indirect_array& sub_indices, Ranking_ClusterInfo&, long excluded_goi = -1);

private:
	std::vector<size_type> genes_of_interest; // genes_of_interest
	std::shared_ptr<Clustering> clustering;
	Rankings final_rankings; // finalised rankings, after ctor has finished
	double ausr;
	std::string name;
	std::unordered_map<const Cluster*, Ranking_ClusterInfo> cluster_info; // TODO could use vector instead, uses just iterate over all of it
};

}
