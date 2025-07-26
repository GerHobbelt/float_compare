
#include "float_compare.hpp"

#include <cassert>
#include <iostream>

using namespace flt;

#if defined(BUILD_MONOLITHIC)
#define main   float_compare_test_main
#endif

extern "C"
int main(void) {
	double d1 = 3.14159;
	double d2 = d1;
	double d2e1 = std::nextafter(d1, std::numeric_limits<double>::lowest());
	double d2e2 = std::nextafter(d1, std::numeric_limits<double>::max());
	double d2e3 = std::nextafter(d2e1, std::numeric_limits<double>::lowest());
	double d2e4 = std::nextafter(d2e2, std::numeric_limits<double>::max());
	double d3 = d1+.0001;

	dAbsEq eq1;
	dAbsEq eq2(.001);

	assert(eq1(d1, d2));
	assert(eq1(d1, d2e1));
	assert(eq1(d1, d2e2));
	assert(eq1(d1, d2e3));
	assert(eq1(d1, d2e4));
	assert(!eq1(d1, d3));
	assert(eq2(d1, d3));

	assert(eq1.same(d1, d2));
	assert(eq1.same(d1, d2e1));
	assert(eq1.same(d1, d2e2));
	assert(!eq1.same(d1, d2e3)); // .same() is *almost* an *identicality* check: it checks +/- 1 std::nextafter distance and thus CORRECTLY FAILS this test, where the distance is *2* (tiny) std::nextafter steps!
	assert(!eq1.same(d1, d2e4));
	assert(!eq1.same(d1, d3));
	assert(!eq2.same(d1, d3)); // .same() does not consider the specified epsilon; it only minds the std::nextafter distance and thus CORRECTLY FAILS this test.

	//----------------------

	float d11 = 3.14159f;
	float d12 = d11;
	float d12e1 = std::nextafter(d11, std::numeric_limits<float>::lowest());
	float d12e2 = std::nextafter(d11, std::numeric_limits<float>::max());
	float d12e3 = std::nextafter(d12e1, std::numeric_limits<float>::lowest());
	float d12e4 = std::nextafter(d12e2, std::numeric_limits<float>::max());
	float d13 = d11+.0001f;

	fAbsEq eq11;
	fAbsEq eq12(.001);

	assert(eq11(d11, d12));
	assert(eq11(d11, d12e1));
	assert(eq11(d11, d12e2));
	assert(eq11(d11, d12e3));
	assert(eq11(d11, d12e4));
	assert(!eq11(d11, d13));
	assert(eq12(d11, d13));

	assert(eq11.same(d11, d12));
	assert(eq11.same(d11, d12e1));
	assert(eq11.same(d11, d12e2));
	assert(!eq11.same(d11, d12e3));
	assert(!eq11.same(d11, d12e4));
	assert(!eq11.same(d11, d13));
	assert(!eq12.same(d11, d13));

	std::cerr << "All tests passed!\n";

	return 0;
}
