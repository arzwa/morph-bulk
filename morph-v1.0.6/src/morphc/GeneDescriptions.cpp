// Author: Tim Diels <timdiels.m@gmail.com>

#include "GeneDescriptions.h"
#include <cctype>
#include <algorithm>
#include <iostream>
#include <assert.h>

using namespace std;

namespace MORPHC {

GeneDescriptions::GeneDescriptions(string path)
:	mapping(path)
{
}

std::string GeneDescriptions::get(std::string gene) const {
	auto& descriptions = mapping.get();
	assert(all_of(gene.begin(), gene.end(), [](int c){ return islower(c); }));
	auto it = descriptions.find(gene);
	if (it == descriptions.end()) {
		//cout << "Warning: description not found for gene: " << gene << "\n";
		return "";
	}
	else {
		return it->second;
	}
}

}
