//
// This file is subject to the terms and conditions defined in 'LICENSE' included in the source code package
//

#ifndef JACTORIO_INCLUDE_CORE_FLOAT_MATH_H
#define JACTORIO_INCLUDE_CORE_FLOAT_MATH_H

#include <cmath>

namespace jactorio::core
{
	// Defines math functions for floating point numbers

	inline bool f_eq(const float a, const float b, const float epsilon) {
		return fabs(a - b) < epsilon;
	}
}

#endif //JACTORIO_INCLUDE_CORE_FLOAT_MATH_H
