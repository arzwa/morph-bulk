/*
 * Copyright (C) 2014 by Tim Diels
 *
 * This file is part of binreverse.
 *
 * binreverse is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * binreverse is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with binreverse.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "util.h"
#include <algorithm>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/archive/archive_exception.hpp>
#include <cctype>

using namespace std;

namespace MORPHC {

void read_file(std::string path, std::function<const char* (const char*, const char*)> reader) {
	try {
		boost::iostreams::mapped_file mmap(path, boost::iostreams::mapped_file::readonly);
		auto begin = mmap.const_data();
		auto end = begin + mmap.size();

		// trim last newline if any
		if (begin != end && *(end-1) == '\n') {
			end--;
			if (begin != end && *(end-1) == '\r')
				end--;
		}

		const char* current = nullptr;
		try {
			current = reader(begin, end);
		}
		catch (const boost::spirit::qi::expectation_failure<const char*>& e) {
			throw runtime_error(exception_what(e)); // get error message here, as outside the outer try block it'd segfault when reading from file (which this one does)
		}

		current = find_if_not(current, end, [](char c){ return isspace(c); }); // skip trailing whitespace
		if (current != end) {
			ostringstream out;
			out << "Trailing characters at end of file: '";
			copy(current, end, ostream_iterator<char>(out));
			out << "'";
			throw runtime_error(out.str());
		}
	}
	catch (const exception& e) {
		throw runtime_error((make_string() << "Error while reading '" << path << "': " << exception_what(e)).str());
	}
}

string prepend_path(string prefix, string path) {
	if (path.at(0) == '/')
		return path;
	else
		return prefix + "/" + path;
}

void ensure(bool condition, std::string error_message, ErrorType error_category) {
	if (!condition)
		throw TypedException(error_message, error_category);
}

/**
 * Prints something of boost spirit
 *
 * Source: http://www.boost.org/doc/libs/1_57_0/libs/spirit/doc/html/spirit/qi/reference/basics.html#spirit.qi.reference.basics.examples
 */
class SpiritPrinter
{
public:
    typedef boost::spirit::utf8_string string;

    SpiritPrinter(ostream& out)
    :	out(out)
    {
    }

    void element(string const& tag, string const& value, int depth) const
    {
        for (int i = 0; i < (depth*4); ++i) // indent to depth
            out << ' ';

        out << "tag: " << tag;
        if (value != "")
            out << ", value: " << value;
        out << "\n";
    }

private:
    ostream& out;
};

std::string exception_what(const exception& e) {
	auto expectation_failure_ex = dynamic_cast<const boost::spirit::qi::expectation_failure<const char*>*>(&e);
	if (expectation_failure_ex) {
		auto& e = *expectation_failure_ex;
		ostringstream str;
		str << e.what() << ": \n";

		str << "Expected: \n";
		SpiritPrinter printer(str);
		boost::spirit::basic_info_walker<SpiritPrinter> walker(printer, e.what_.tag, 0);
		boost::apply_visitor(walker, e.what_.value);

		str << "Got: \"" << std::string(e.first, e.last) << '"' << endl;

		return str.str();
	}

	auto ios_clear_ex = dynamic_cast<const std::ios::failure*>(&e);
	if (ios_clear_ex) {
		return (make_string() << ios_clear_ex->what() << ": " << strerror(errno)).str();
	}


	auto archive_ex = dynamic_cast<const boost::archive::archive_exception*>(&e);
	if (archive_ex) {
		return (make_string() << archive_ex->what() << ": " << strerror(errno)).str();
	}

	return e.what();
}

void to_lower(std::string& data) {
	std::transform(data.begin(), data.end(), data.begin(), ::tolower);
}

}
