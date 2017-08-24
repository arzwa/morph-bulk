// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <string>
#include <unordered_map>
#include <boost/noncopyable.hpp>

namespace MORPHC {

// TODO binary serialize
// TODO lower(gene) -> string* is what this really is
/**
 * Mapping of string to string
 */
class StringMapping : public boost::noncopyable {
public:
	/**
	 * Load tab delimited file with 2 columns
	 */
	StringMapping(std::string path);

	const std::unordered_map<std::string, std::string>& get() const;

private:
	std::unordered_map<std::string, std::string> mapping;
};

}
