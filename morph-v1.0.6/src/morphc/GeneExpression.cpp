// Author: Tim Diels <timdiels.m@gmail.com>

#include "GeneExpression.h"
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/spirit/include/qi.hpp>
#include <gsl/gsl_statistics.h>
#include <cmath>
#include <iomanip>
#include "util.h"
#include <morphc/TabGrammarRules.h>

using namespace std;
namespace ublas = boost::numeric::ublas;

namespace MORPHC {

GeneExpression::GeneExpression(string data_root, const YAML::Node node, Cache& cache)
{
	name = node["name"].as<string>();

	// load expression_matrix
	cache.load_bin_or_plain(prepend_path(data_root, node["path"].as<string>()), *this);
}

void GeneExpression::load_plain(std::string path) {
	int j;
	read_file(path, [this, &j](const char* begin, const char* end) {
		using namespace boost::spirit::qi;

		auto current = begin;

		// count lines in file
		int line_count = count(begin, end, '\n') + 1; // #lines = #line_separators + 1

		// parse header
		std::vector<std::string> header_items;
		TabGrammarRules rules;
		parse(current, end, rules.line > eol, header_items);

		// resize matrix
		expression_matrix.resize(line_count-1, header_items.size()-1, false);

		// parse gene lines
		int i=-1;
		j=-1;
		auto on_new_gene = [this, &i, &j](std::string name) { // start new line
			to_lower(name);
			ensure(i<0 || j==expression_matrix.size2()-1, (
					make_string() << "Line " << i+2 << " (1-based, header included): expected "
					<< expression_matrix.size2() << " columns, got " << j+1).str(),
					ErrorType::GENERIC
			);
			i++;
			ensure(gene_indices.emplace(name, i).second,
					(make_string() << "Duplicate gene: " << name).str(),
					ErrorType::GENERIC);
			gene_names.emplace(i, name);
			genes.push_back(i);
			j=-1;
		};
		auto on_gene_value = [this, &i, &j](double value) { // gene expression value
			j++;
			expression_matrix(i, j) = value;
		};

		parse(current, end, (rules.field[on_new_gene] > rules.separator > (double_[on_gene_value] % rules.separator)) % eol);

		return current;
	});
	ensure(j == expression_matrix.size2()-1,
			(make_string() << "Error while reading " << path << ": Incomplete line: " << j+1 << " values instead of " << expression_matrix.size2()).str(),
			ErrorType::GENERIC);
}

void GeneExpression::generate_gene_correlations(const std::vector<size_type>& all_goi) {
	using namespace ublas;
	MORPHC::indirect_array goi_indices(const_cast<size_type*>(&*all_goi.begin()), const_cast<size_type*>(&*all_goi.end()));

	for (size_type i=0; i<all_goi.size(); i++) {
		gene_column_indices[all_goi.at(i)] = i;
	}
	gene_correlations = GeneCorrelations(expression_matrix.size1(), all_goi.size());

	// calculate Pearson's correlation
	// This is gsl_stats_correlation's algorithm, but in matrix form.
	size_type i;
	ublas::vector<long double> mean(expression_matrix.size1());
	ublas::vector<long double> delta(expression_matrix.size1());
	ublas::vector<long double> sum_sq(expression_matrix.size1(), 0.0); // sum of squares
	ublas::matrix<long double> sum_cross(expression_matrix.size1(), goi_indices.size(), 0.0);

	mean = column(expression_matrix, 0);

	for (i = 1; i < expression_matrix.size2(); ++i)
	{
		long double ratio = i / (i + 1.0);
		noalias(delta) = column(expression_matrix, i) - mean;
		sum_sq += element_prod(delta, delta) * ratio;
		sum_cross += outer_prod(delta, project(delta, goi_indices)) * ratio;
		mean += delta / (i + 1.0);
	}

	transform(sum_sq.begin(), sum_sq.end(), sum_sq.begin(), ::sqrt);
	gene_correlations = element_div(sum_cross, outer_prod(sum_sq, project(sum_sq, goi_indices)));
}

const GeneCorrelations& GeneExpression::get_gene_correlations() const {
	return gene_correlations;
}

size_type GeneExpression::get_gene_index(std::string name) const {
	assert(has_gene(name));
	return gene_indices.find(name)->second;
}

std::string GeneExpression::get_gene_name(size_type index) const {
	assert(gene_names.find(index) != gene_names.end());
	return gene_names.find(index)->second;
}

bool GeneExpression::has_gene(string gene) const {
	return gene_indices.find(gene) != gene_indices.end();
}

size_type GeneExpression::get_gene_column_index(size_type gene_row_index) const {
	return gene_column_indices.at(gene_row_index);
}

string GeneExpression::get_name() const {
	return name;
}

const std::vector<size_type>& GeneExpression::get_genes() const {
	return genes;
}

void GeneExpression::dispose_expression_data() {
	expression_matrix.resize(0, 0);
}

void GeneExpression::dispose_correlations() {
	gene_correlations.resize(0, 0);
}

}
