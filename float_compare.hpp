// Inspired by CoinFloatEqual.hpp, Copyright (C) 2000, International Business Machines Corporation and others.  All Rights Reserved.
// This code is licensed under the terms of the Eclipse Public License (EPL).

#ifndef FLOAT_COMPARE_LIB_BARRIER
#define FLOAT_COMPARE_LIB_BARRIER

#include <limits>
#include <cfloat>
#include <cmath>

namespace flt {

	// https://stackoverflow.com/questions/570669/checking-if-a-double-or-float-is-nan-in-c :
	// std::isnan() doesn't deliver under GCC.

	/*
	On x86-64 you can have extremely fast methods for checking for NaN and infinity, which work regardless of -ffast-math compiler option. (f != f, std::isnan, std::isinf always yield false with -ffast-math).

	Testing for NaN, infinity and finite numbers can easily be done by checking for maximum exponent. infinity is maximum exponent with zero mantissa, NaN is maximum exponent and non-zero mantissa. The exponent is stored in the next bits after the topmost sign bit, so that we can just left shift to get rid of the sign bit and make the exponent the topmost bits, no masking (operator&) is necessary:

																														-- Maxim Egorushkin
	*/

	static inline uint64_t load_ieee754_rep(double a) {
		uint64_t r;
		static_assert(sizeof r == sizeof a, "Unexpected sizes.");
		std::memcpy(&r, &a, sizeof a); // Generates movq instruction.
		return r;
	}

	static inline uint32_t load_ieee754_rep(float a) {
		uint32_t r;
		static_assert(sizeof r == sizeof a, "Unexpected sizes.");
		std::memcpy(&r, &a, sizeof a); // Generates movd instruction.
		return r;
	}

	constexpr uint64_t inf_double_shl1 = UINT64_C(0xffe0000000000000);
	constexpr uint32_t inf_float_shl1 = UINT32_C(0xff000000);

	// The shift left removes the sign bit. The exponent moves into the topmost bits,
	// so that plain unsigned comparison is enough.
	static inline bool isnan2(double a) { return load_ieee754_rep(a) << 1  > inf_double_shl1; } // Both NaN and qNan.
	static inline bool isinf2(double a) { return load_ieee754_rep(a) << 1 == inf_double_shl1; }
	static inline bool isfinite2(double a) { return load_ieee754_rep(a) << 1  < inf_double_shl1; }
	static inline bool isnan2(float a) { return load_ieee754_rep(a) << 1  > inf_float_shl1; } // Both NaN and qNan.
	static inline bool isinf2(float a) { return load_ieee754_rep(a) << 1 == inf_float_shl1; }
	static inline bool isfinite2(float a) { return load_ieee754_rep(a) << 1  < inf_float_shl1; }


template <typename T, double default_epsilon_factor>
class cmp {
public:
	//=============================================================================
	// Smallest positive floating point value and Plus infinity (double and int)

	static constexpr const double FLT_VAL_MIN = (std::numeric_limits< T >::min)();
	static constexpr const double FLT_VAL_MAX = (std::numeric_limits< T >::max)();
	static constexpr const int INT_VAL_MAX = (std::numeric_limits< int >::max)();
	static constexpr const double INT_MAX_AS_DOUBLE = INT_VAL_MAX;
	static constexpr const double FLT_VAL_EPSILON = (std::numeric_limits< T >::epsilon)();

	/** checks if a double value is not a number */
	static inline bool IsNaN(T val)
	{
		// https://stackoverflow.com/questions/570669/checking-if-a-double-or-float-is-nan-in-c :
		// std::isnan() doesn't deliver under GCC.
		return isnan2(val);
	}

	/** checks if a double value is infinite (and not NaN) */
	static inline bool IsInfinite(T val)
	{
		return isinf2(val) || val == FLT_VAL_MAX || val == -FLT_VAL_MAX;
	}

	/** checks if a double value is finite (not infinity and not NaN) */
	static inline bool IsFinite(T val)
	{
		return isfinite2(val) && val != FLT_VAL_MAX && val != -FLT_VAL_MAX;
	}


	/*! \brief Function objects for testing equality of real numbers.

		Two objects are provided; one tests for equality to an absolute tolerance,
		one to a scaled tolerance. The tests will handle IEEE floating point, but
		note that infinity == infinity when you ask `same()`. Mathematicians are rolling in their graves,
		but this matches the behaviour for the common practice of using
		<code>DBL_MAX</code> (<code>numeric_limits<double>::max()</code>, or similar
		large finite number) as infinity.

		<p>
		Example usage:
		@verbatim
			double d1 = 3.14159 ;
			double d2 = d1 ;
			double d3 = d1+.0001 ;

			CoinAbsFltEq eq1 ;
			CoinAbsFltEq eq2(.001) ;

			assert(  eq1(d1,d2) ) ;
			assert( !eq1(d1,d3) ) ;
			assert(  eq2(d1,d3) ) ;
		@endverbatim
		CoinRelFltEq follows the same pattern.
	*/

	/*! \brief Equality to an absolute tolerance

		Operands are considered equal if their difference is within an epsilon ;
		the test does not consider the relative magnitude of the operands.
	*/

	class AbsEq
	{
	public:

		// https://stackoverflow.com/questions/13698927/compare-double-to-zero-using-epsilon
		// 
		// The difference between X and the next value of X varies according to X.
		// epsilon() is only the difference between 1 and the next value of 1.
		// The difference between 0 and the next value of 0 is not epsilon().
		//
		// The programmer probably rather wants some small threshold to test for equality, e.g. +/-1e-6 or +/-1e-9, instead of +/-nextafter().

	//! Compare function

		inline bool operator()(const T f1, const T f2) const
		{
			//if (IsNaN(f1) || IsNaN(f2))
			//  return false;
			if (!IsFinite(f1) || !IsFinite(f2))
				return false;
			if (f1 == f2)
				return true;
			return (fabs(f1 - f2) < epsilon_);
		}

		// https://stackoverflow.com/questions/13698927/compare-double-to-zero-using-epsilon

		inline bool same(T f1, T f2)
		{
			if (IsNaN(f1) || IsNaN(f2))
				return IsNaN(f1) == IsNaN(f2);
			if (!IsFinite(f1) || !IsFinite(f2))
				return IsFinite(f1) == IsFinite(f2);
			if (f1 == f2)
				return true;

			return std::nextafter(f1, std::numeric_limits<T>::lowest()) <= f2
				&& std::nextafter(f1, std::numeric_limits<T>::max()) >= f2;
		}

		/*! \name Constructors and destructors */
		//@{

		/*! \brief Default constructor

			Default tolerance is 1.0e-10.
		*/

		AbsEq(): epsilon_(T(default_epsilon_factor))
		{
		}

		//! Alternate constructor with epsilon as a parameter

		AbsEq(const T epsilon): epsilon_(epsilon)
		{
			if (epsilon_ < FLT_VAL_EPSILON)
				epsilon_ = FLT_VAL_EPSILON;
		}

		//! Destructor

		~AbsEq() = default;

		//! Copy constructor

		AbsEq(const AbsEq &src): epsilon_(src.epsilon_)
		{
		}

		//! Assignment

		AbsEq &operator=(const AbsEq &rhs)
		{
			if (this != &rhs)
				epsilon_ = rhs.epsilon_;
			return (*this);
		}

		//@}

	private:
		/*! \name Private member data */
		//@{

		//! Equality tolerance.

		T epsilon_;

		//@}
	};

	/*! \brief Equality to a scaled tolerance

		Operands are considered equal if their difference is within a scaled
		epsilon calculated as epsilon_ * std::max(|f1|,|f2|).
	*/

	class RelEq
	{
	public:

		//! Compare function

		inline bool operator()(const T f1, const T f2) const
		{
			//if (IsNaN(f1) || IsNaN(f2))
			//  return false;
			if (!IsFinite(f1) || !IsFinite(f2))
				return false;
			if (f1 == f2)
				return true;

			T tol = (fabs(f1) > fabs(f2)) ? fabs(f1) : fabs(f2);
			tol *= epsilon_;
			if (tol < FLT_VAL_EPSILON)
				tol = FLT_VAL_EPSILON;

			return (fabs(f1 - f2) <= tol);
		}

		// https://stackoverflow.com/questions/13698927/compare-double-to-zero-using-epsilon

		inline bool same(T f1, T f2)
		{
			if (IsNaN(f1) || IsNaN(f2))
			  return IsNaN(f1) == IsNaN(f2);
			if (!IsFinite(f1) || !IsFinite(f2))
				return IsFinite(f1) == IsFinite(f2);
			if (f1 == f2)
				return true;

			return std::nextafter(f1, std::numeric_limits<T>::lowest()) <= f2
				&& std::nextafter(f1, std::numeric_limits<T>::max()) >= f2;
		}

		/*! \name Constructors and destructors */
		//@{

		/*! Default constructor

			Default tolerance is 1.0e-10.
		*/
		RelEq(): epsilon_(T(default_epsilon_factor))
		{
		}

		//! Alternate constructor with epsilon as a parameter

		RelEq(const T epsilon): epsilon_(epsilon)
		{
			if (epsilon_ < FLT_VAL_EPSILON)
				epsilon_ = FLT_VAL_EPSILON;
		}

		//! Destructor

		~RelEq() = default;

		//! Copy constructor

		RelEq(const AbsEq &src): epsilon_(src.epsilon_)
		{
		}

		//! Assignment

		RelEq &operator=(const RelEq &rhs)
		{
			if (this != &rhs)
				epsilon_ = rhs.epsilon_;
			return (*this);
		}

		//@}

	private:
		/*! \name Private member data */
		//@{

		//! Equality tolerance.

		T epsilon_;

		//@}
	};
};

// instantiate template for both floating point types: float and double
using fcmp = cmp<float, 1.0e-6>;
using dcmp = cmp<double, 1.0e-10>;

using fAbsEq = fcmp::AbsEq;
using fRelEq = fcmp::RelEq;

using dAbsEq = dcmp::AbsEq;
using dRelEq = dcmp::RelEq;

}

#endif
