
#include "float_compare.hpp"

#include <cassert>

using namespace flt;

#if defined(BUILD_MONOLITHIC)
#define main   float_compare_test_main
#endif

extern "C"
int main() {
	double d1 = 3.14159;
	double d2 = d1;
	double d2e1 = d1 + std::nextafter(d1, std::numeric_limits<double>::lowest());
	double d2e2 = d1 + std::nextafter(d1, std::numeric_limits<double>::max());
	double d3 = d1+.0001;

	dAbsEq eq1;
	dAbsEq eq2(.001);

	assert(eq1(d1, d2));
	assert(eq1(d1, d2e1));
	assert(eq1(d1, d2e2));
	assert(!eq1(d1, d3));
	assert(eq2(d1, d3));

	//----------------------

	float d11 = 3.14159f;
	float d12 = d11;
	float d12e1 = d11 + std::nextafter(d11, std::numeric_limits<float>::lowest());
	float d12e2 = d11 + std::nextafter(d11, std::numeric_limits<float>::max());
	float d13 = d11+.0001f;

	fAbsEq eq11;
	fAbsEq eq12(.001);

	assert(eq11(d11, d12));
	assert(eq11(d11, d12e1));
	assert(eq11(d11, d12e2));
	assert(!eq11(d11, d13));
	assert(eq12(d11, d13));

	return 0;
}
