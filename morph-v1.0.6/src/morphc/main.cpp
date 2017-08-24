// Author: Tim Diels <timdiels.m@gmail.com>

#include <iostream>
#include "Application.h"
#include "util.h"

// TODO fiddling with matrix orientation, would it help performance?
// TODO perhaps not all asserts should be disabled at runtime (e.g. input checks may want to remain)

/**
 * General notes about the code:
 * - goi := genes of interest (always plural, thus not: gene of interest)
 * - gois := goi groups = a collection of goi ~= vector<vector<gene of interest>>
 */

using namespace std;

int main(int argc, char** argv) {
	using namespace MORPHC;
	try {
		Application app(argc, argv);
		app.run();
	}
	catch (const TypedException& e) {
		cerr << "Exception: " << e.what() << endl;
		return e.get_exit_code();
	}
	catch (const exception& e) {
		cerr << "Exception: " << exception_what(e) << endl;
		return static_cast<int>(ErrorType::GENERIC);
	}
	return 0;
}
