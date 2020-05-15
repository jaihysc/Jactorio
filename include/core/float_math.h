// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#ifndef JACTORIO_INCLUDE_CORE_FLOAT_MATH_H
#define JACTORIO_INCLUDE_CORE_FLOAT_MATH_H
#pragma once

#include <cmath>

namespace jactorio::core
{
	// Defines math functions for floating point numbers

	inline bool FEq(const float a, const float b, const float epsilon) {
		return fabs(a - b) < epsilon;
	}
}

#endif //JACTORIO_INCLUDE_CORE_FLOAT_MATH_H
