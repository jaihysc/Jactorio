#ifndef CORE_FLOAT_MATH_H
#define CORE_FLOAT_MATH_H

#include <cmath>

namespace jactorio::core
{
	// Defines math functions for floating point numbers

	constexpr float epsilon = 0.00005;  // Epsilon may need to be adjusted, it cannot seem to reach the precision
										 // specified in FLT_EPSILON

	inline bool f_eq(const float a, const float b) {
		return fabs(a - b) < epsilon;
	}
}

#endif // CORE_FLOAT_MATH_H
