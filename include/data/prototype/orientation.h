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

	inline int InvertOrientation(const int orientation) {
		assert(0 <= orientation && orientation <= 3);
		return (orientation + 2) % 4;
	}

	inline Orientation InvertOrientation(Orientation orientation) {
		return static_cast<Orientation>(
			InvertOrientation(static_cast<int>(orientation))
		);
	}

	///
	/// \brief Increments or decrements Ty var depending on orientation.
	/// up--, right++, down++, left--
	/// \remark Type must implement operator+= and operator-=
	/// \remark if incrementer type not provided, Typeof x will bw used
	template <typename TyX, typename TyY, typename TyInc = TyX>
	void OrientationIncrement(const Orientation orientation,
	                          TyX& x, TyY& y, TyInc increment = 1) {
		switch (orientation) {
		case Orientation::up:
			y -= increment;
			break;
		case Orientation::right:
			x += increment;
			break;
		case Orientation::down:
			y += increment;
			break;
		case Orientation::left:
			x -= increment;
			break;

		default:
			assert(false);
			break;
		}
	}
}

#endif // JACTORIO_DATA_PROTOTYPE_PLACEMENT_ORIENTATION_H
