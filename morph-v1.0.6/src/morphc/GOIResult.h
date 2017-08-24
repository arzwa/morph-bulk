// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <morphc/Ranking.h>

namespace MORPHC {

/**
 * Ranking for GOI
 */
class GOIResult : public boost::noncopyable
{
public:
	GOIResult()
	:	ausr_sum(0.0), ausr_count(0.0)
	{
	}

	void add_ausr(double ausr) {
		ausr_sum += ausr;
		ausr_count++;
	}

	double get_average_ausr() {
		return ausr_sum / ausr_count; // TODO numerically stable mean
	}

public:
	std::unique_ptr<Ranking> best_ranking;

private:
	double ausr_sum; // sum of all ausr
	double ausr_count; // number of AUSRs added
};

}
