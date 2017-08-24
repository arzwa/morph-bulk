// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <string>
#include <unordered_map>
#include <map>

namespace MORPHC {

// Note: when changing enum, be sure to update list in usage printing of Application.cpp
enum class ErrorType : int {
	NONE = 0,
	GENERIC,
	INVALID_GOI_GENE
};

// XXX in a normal world one'd use inheritance to add types..., derived types...
class TypedException : public std::runtime_error
{
public:
	TypedException(std::string what, ErrorType error_type)
	:	std::runtime_error(what), error_type(error_type)
	{
	}

	int get_exit_code() const {
		return static_cast<int>(error_type);
	}

private:
	ErrorType error_type;
};

} // end MORPHC
