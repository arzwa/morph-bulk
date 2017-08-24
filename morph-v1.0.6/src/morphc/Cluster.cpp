// Author: Tim Diels <timdiels.m@gmail.com>

#include "Cluster.h"
#include <morphc/util.h>

using namespace std;
namespace ublas = boost::numeric::ublas;

namespace MORPHC {

Cluster::Cluster(string name)
:	name(name)
{
}

void Cluster::add(size_type gene_index) {
	genes.emplace_back(gene_index);
}

bool Cluster::empty() const {
	return genes.empty();
}

std::vector<size_type>::const_iterator Cluster::begin() const {
	return genes.begin();
}

std::vector<size_type>::const_iterator Cluster::end() const {
	return genes.end();
}

std::vector<size_type>::iterator Cluster::begin() {
	return genes.begin();
}

std::vector<size_type>::iterator Cluster::end() {
	return genes.end();
}

string Cluster::get_name() const {
	return name;
}

}
