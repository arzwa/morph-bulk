// Author: Tim Diels <timdiels.m@gmail.com>

#include "Ranking.h"
#include "util.h"
#include "gsl/gsl_statistics_double.h"
#include <fstream>
#include <iomanip>
#include <cmath>
#include <boost/algorithm/string.hpp>
#include <errno.h>

using namespace std;
namespace ublas = boost::numeric::ublas;
using namespace ublas;

namespace MORPHC {

/**
 * One row of a ranking
 */
class Rank
{
public:
	Rank(int rank, string gene, double score, string annotation, string gene_web_page)
	:	rank(rank), gene(gene), score(score), annotation(annotation), gene_web_page(gene_web_page)
	{
	}

public:
	int rank; // row in ranking, 1-based
	string gene;
	double score;
	string annotation;
	string gene_web_page;
};


size_type K = 1000;

Ranking_ClusterInfo::Ranking_ClusterInfo(const GeneExpression& gene_expression, const std::vector<size_type>& genes_of_interest, const Cluster& c)
{
	auto& cluster = const_cast<Cluster&>(c);
	auto is_goi = [&genes_of_interest](size_type gene) {
		return contains(genes_of_interest, gene);
	};
	auto candidates_begin = partition(cluster.begin(), cluster.end(), is_goi); // Note: modifying the order of cluster genes doesn't really change the cluster

	goi_ = MORPHC::array(distance(cluster.begin(), candidates_begin));
	copy(cluster.begin(), candidates_begin, goi_.begin());
	goi = MORPHC::indirect_array(goi_.size(), goi_); // Note: ublas indirect_array is making me do ugly things

	candidates = MORPHC::indirect_array(&*candidates_begin, &*cluster.end());
	genes = MORPHC::indirect_array(&*cluster.begin(), &*cluster.end());

	goi_columns_ = MORPHC::array(goi.size());
	for (size_type i=0; i<goi.size(); i++) {
		goi_columns_[i] = gene_expression.get_gene_column_index(goi_[i]);
	}
	goi_columns = MORPHC::indirect_array(goi_columns_.size(), goi_columns_);
}

Ranking::Ranking(std::vector<size_type> goi, std::shared_ptr<Clustering> clustering, std::string name)
:	genes_of_interest(goi), clustering(clustering), ausr(-1.0), name(name)
{
	Rankings rankings(clustering->get_source().get_gene_correlations().size1(), nan("undefined"));

	// fill rankings with intermediary values
	rank_genes(goi, rankings);

	// finish calculation of rankings
	final_rankings = Rankings(rankings.size(), nan("undefined")); // TODO we might be able to drop the nan init and just have garble in it (everything gets assigned a new value I think)
	finalise_ranking(rankings);

	// calculate ausr
	rank_self(rankings);
}

void Ranking::rank_genes(const std::vector<size_type>& genes_of_interest, Rankings& rankings) {
	for (auto& cluster : *clustering) {
		auto& info = cluster_info.emplace(piecewise_construct, make_tuple(&cluster), make_tuple(std::ref(get_gene_expression()), std::ref(genes_of_interest), std::ref(cluster))).first->second;

		// skip if no goi or candidates in this cluster
		if (info.get_goi_count() == 0 || info.candidates.size() == 0)
			continue;

		// compute rankings
		auto sub_matrix = project(get_gene_correlations(), info.genes, info.goi_columns);
		auto sub_rankings = project(rankings, info.genes);
		noalias(sub_rankings) = prod(sub_matrix, ublas::scalar_vector<double>(info.get_goi_count())); // TODO might want to consider more numerically stable kind of mean
	}
}

void Ranking::finalise_ranking(const Rankings& rankings) {
	for (auto& cluster : *clustering) {
		auto& info = cluster_info.at(&cluster);
		finalise_sub_ranking(rankings, final_rankings, info.candidates, info);
	}
}

void Ranking::finalise_sub_ranking(const Rankings& rankings, Rankings& final_rankings, const MORPHC::indirect_array& sub_indices, Ranking_ClusterInfo& info, long excluded_goi) {
	if (info.get_goi_count() == 0 || info.candidates.size() == 0) {
		return; // in this case, all values in these ranking will (and should) be NaN
	}

	auto sub_rankings = project(rankings, sub_indices);
	auto final_sub_rankings = project(final_rankings, sub_indices);
	if (excluded_goi > 0) {
		auto sub_matrix = project(column(get_gene_correlations(), get_gene_expression().get_gene_column_index(excluded_goi)), sub_indices);
		noalias(final_sub_rankings) = (sub_rankings - sub_matrix) / (info.get_goi_count() - 1);
	}
	else {
		noalias(final_sub_rankings) = sub_rankings / info.get_goi_count();
	}

	// unset partial ranking of goi genes in final ranking
	for (auto gene : info.goi) {
		if (gene != excluded_goi) {
			final_rankings(gene) = nan("undefined");
		}
	}

	// Normalise scores within this cluster: uses GSL => numerically stable
	std::vector<double> sub_ranks(final_sub_rankings.begin(), final_sub_rankings.end());
	double mean_ = gsl_stats_mean(sub_ranks.data(), 1, sub_ranks.size());
	double standard_deviation_ = gsl_stats_sd_m(sub_ranks.data(), 1, sub_ranks.size(), mean_);
	final_sub_rankings = (final_sub_rankings - ublas::scalar_vector<double>(final_sub_rankings.size(), mean_)) / standard_deviation_;

	// Normalise scores within this cluster: perhaps numerically unstable TODO
	/*auto mean = ublas::inner_prod(sub_rankings, ublas::scalar_vector<double>(sub_rankings.size())) / sub_rankings.size();
	sub_rankings = sub_rankings - ublas::scalar_vector<double>(sub_rankings.size(), mean);
	auto standard_deviation = ublas::norm_2(sub_rankings) / sqrt(sub_rankings.size()-1); // TODO could we pass the iterator to gsl_stats_sd and such?. If not at least use gsl_*sqrt
	//sub_rankings = sub_rankings / standard_deviation;*/
}

void Ranking::rank_self(const Rankings& rankings) {
	// find rank_indices of leaving out a gene of interest one by one
	std::vector<size_type> rank_indices;
	Rankings final_rankings = this->final_rankings;
	for (auto& p : cluster_info) {
		auto& cluster = *p.first;
		auto& info = p.second;
		MORPHC::array candidates_and_gene_(info.candidates.size() + 1);
		copy(info.candidates.begin(), info.candidates.end(), candidates_and_gene_.begin());
		for (auto gene : info.goi) {
			candidates_and_gene_[candidates_and_gene_.size()-1] = gene;
			MORPHC::indirect_array candidates_and_gene(candidates_and_gene_.size(), candidates_and_gene_);
			finalise_sub_ranking(rankings, final_rankings, candidates_and_gene, info, gene);

			double rank = final_rankings(gene);
			if (std::isnan(rank)) {
				// gene undetected, give penalty
				rank_indices.emplace_back(2*K-1); // Note: anything >=K doesn't count towards the AUSR
			}
			else {
				// TODO you can stop if you notice it's >=K
				// TODO currently we do len(GOI) passes on the whole ranking, a sort + single pass is probably faster
				size_type count = count_if(final_rankings.begin(), final_rankings.end(), [rank](double val){return val > rank && !std::isnan(val);});
				rank_indices.emplace_back(count);
			}
		}
		project(final_rankings, info.genes) = project(this->final_rankings, info.genes); // restore what we changed
	}
	ensure(rank_indices.size() == genes_of_interest.size(), "Assertion failed: rank_indices.size() == genes_of_interest.size()", ErrorType::GENERIC);
	sort(rank_indices.begin(), rank_indices.end());

	// calculate ausr
	double auc = 0.0; // area under curve
	for (size_type i = 0; i < K; i++) {
		// TODO can continue last search for upper bound where we left last one...
		auto supremum = upper_bound(rank_indices.begin(), rank_indices.end(), i);
		auto count = distance(rank_indices.begin(), supremum);
		auc += (double)count / rank_indices.size();
	}
	ausr = auc / K;
}

void Ranking::save(std::string path, int top_k, const GeneDescriptions& descriptions, std::string gene_web_page_template, const GenesOfInterest& full_goi, double average_ausr, bool output_yaml) {
	// Sort results
	std::vector<pair<double, string>> results; // vec<(rank, gene)>
	auto& gene_expression = clustering->get_source();
	for (int i=0; i<final_rankings.size(); i++) {
		if (std::isnan(final_rankings(i)))
			continue; // don't include unranked genes in results
		results.push_back(make_pair(final_rankings(i), gene_expression.get_gene_name(i)));
	}
	sort(results.rbegin(), results.rend());

	// Gather output data
	string gene_expression_name = clustering->get_source().get_name();
	string clustering_name = clustering->get_name();

	std::vector<string> goi_genes_present; // note: this is always non-empty
	for (auto g : genes_of_interest) {
		goi_genes_present.emplace_back(get_gene_expression().get_gene_name(g));
	}

	std::vector<string> goi_genes_missing;
	for (auto g : full_goi.get_genes()) {
		if (!get_gene_expression().has_gene(g)) {
			goi_genes_missing.emplace_back(g);
		}
	}

	std::vector<Rank> ranks;
	for (int i=0; i<results.size() && i<top_k; i++) {
		auto& r = results.at(i);
		auto& gene = r.second;
		assert(!std::isnan(r.first));
		std::string web_page = gene_web_page_template;
		boost::replace_all(web_page, "$name", gene);
		ranks.emplace_back(i+1, gene, r.first, descriptions.get(gene), web_page);
	}

	// Out put results
	ofstream out(path + "/" + name);
	out.exceptions(ofstream::failbit | ofstream::badbit);

	if (output_yaml) {
		YAML::Node ranking;
		ranking["best_ausr"] = ausr;
		ranking["average_ausr"] = average_ausr;
		ranking["gene_expression_name"] = gene_expression_name;
		ranking["clustering_name"] = clustering_name;
		ranking["goi_genes_present"] = goi_genes_present;
		ranking["goi_genes_missing"] = goi_genes_missing;
		for (auto& rank : ranks) {
			YAML::Node candidate;
			candidate["rank"] = rank.rank;
			candidate["gene"] = rank.gene;
			candidate["score"] = rank.score;
			candidate["annotation"] = rank.annotation;
			candidate["gene_web_page"] = rank.gene_web_page;
			ranking["candidates"].push_back(candidate);
		}

		YAML::Node root;
		root["ranking"] = ranking;
		out << YAML::Dump(root);
	}
	else { // plain text output
		out << setprecision(2) << fixed;
		out << "Best AUSR: " << ausr << "\n"; // Note: "\n" is faster to out put than std::endl
		out << "Average AUSR: " << average_ausr << "\n";
		out << "Gene expression data set: " << gene_expression_name << "\n";
		out << "Clustering: " << clustering_name << "\n";

		out << "Genes of interest present in data set: ";
		copy(goi_genes_present.begin(), goi_genes_present.end(), ostream_iterator<string>(out, " "));
		out << "\n";

		if (!goi_genes_missing.empty()) {
			out << "Genes of interest missing in data set: ";
			copy(goi_genes_missing.begin(), goi_genes_missing.end(), ostream_iterator<string>(out, " "));
			out << "\n";
		}

		out << "\n";
		out << "Candidates:\n";
		out << "Rank\tGene ID\tScore\tAnnotation\tGene web page\n";
		for (auto& rank : ranks) {
			out << rank.rank << "\t" << rank.gene << "\t" << rank.score << "\t" << rank.annotation << "\t" << rank.gene_web_page << "\n";
		}
	}
}

bool Ranking::operator>(const Ranking& other) const {
	return ausr > other.ausr;
}

double Ranking::get_ausr() const {
	return ausr;
}

const GeneCorrelations& Ranking::get_gene_correlations() {
	return get_gene_expression().get_gene_correlations();
}

const GeneExpression& Ranking::get_gene_expression() {
	return clustering->get_source();
}

}
