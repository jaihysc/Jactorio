// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 04/23/2020

#ifndef JACTORIO_DATA_PROTOTYPE_PLACEMENT_ORIENTATION_H
#define JACTORIO_DATA_PROTOTYPE_PLACEMENT_ORIENTATION_H
#pragma once

#include "jactorio.h"

namespace jactorio::data
{
	enum class Orientation
	{
		up = 0,
		right,
		down,
		left
	};

	inline int invert_orientation(const int orientation) {
		assert(0 <= orientation && orientation <= 3);
		return (orientation + 2) % 4; 
	}

	inline Orientation invert_orientation(Orientation orientation) {
		return static_cast<Orientation>(
			invert_orientation(static_cast<int>(orientation))
		); 
	}
}

#endif // JACTORIO_DATA_PROTOTYPE_PLACEMENT_ORIENTATION_H
