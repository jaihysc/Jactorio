#ifndef CORE_FLOAT_MATH_H
#define CORE_FLOAT_MATH_H

#include <cmath>

namespace jactorio::core
{
	// Defines math functions for floating point numbers

	// Epsilon may need to be adjusted, it cannot seem to reach the precision
	// specified in FLT_EPSILON
	constexpr float transport_line_epsilon = 0.0001;  // was 0.00005

	inline bool f_eq(const float a, const float b, const float epsilon) {
		return fabs(a - b) < epsilon;
	}
}

#endif // CORE_FLOAT_MATH_H
