// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <string>
#include <morphc/StringMapping.h>
#include <boost/noncopyable.hpp>

namespace MORPHC {

class GeneDescriptions : public boost::noncopyable {
public:
	GeneDescriptions(std::string path);

	/**
	 * Get description of gene
	 */
	std::string get(std::string gene) const;

private:
	StringMapping mapping; // gene -> description
};

}
