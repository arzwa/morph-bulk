// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <vector>
#include <map>
#include <utility>
#include <unordered_set>
#include <boost/noncopyable.hpp>
#include <morphc/Cache.h>
#include "GenesOfInterest.h"
#include "ublas.h"

namespace MORPHC {

class Cluster //TODO: public boost::noncopyable
{
public:
	Cluster() {}  // boost::serialization uses this to construct an invalid Cluster before loading it
	Cluster(std::string name);

	void add(size_type index);

	/**
	 * Get whether cluster is empty
	 */
	bool empty() const;

	/**
	 * Get iterator to first gene
	 */
	std::vector<size_type>::const_iterator begin() const;
	std::vector<size_type>::const_iterator end() const;
	std::vector<size_type>::iterator begin();
	std::vector<size_type>::iterator end();

	std::string get_name() const;

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version);

private:
	std::vector<size_type> genes;
	std::string name;

private:
	friend class boost::serialization::access;
};


/////////////////////
// hpp

template<class Archive>
void Cluster::serialize(Archive& ar, const unsigned int version) {
	ar & name;
	ar & genes;
}

} // end MORPHC
