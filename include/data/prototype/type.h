// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_DATA_PROTOTYPE_TYPE_H
#define JACTORIO_DATA_PROTOTYPE_TYPE_H
#pragma once

#include <decimal.h>
#include <type_traits>

#include "jactorio.h"

namespace jactorio::data
{
	using ProtoFloatT = double;
	using ProtoIntT = int32_t;
	using ProtoUintT = uint32_t;

	// Common types used by prototypes, includes no jactorio files

	constexpr auto kInserterRotationDecimals = 3;

	// Defines types for prototype classes
	using RotationDegree = dec::decimal<kInserterRotationDecimals>;

	J_NODISCARD inline RotationDegree ToRotationDegree(const double val) {
		return dec::decimal_cast<kInserterRotationDecimals>(val);
	}

	//

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
		static_assert(std::is_same<TyX, TyY>::value);
		static_assert(std::is_signed<TyX>::value);

		switch (orientation) {
		case Orientation::up:
			y -= static_cast<TyX>(increment);
			break;
		case Orientation::right:
			x += static_cast<TyX>(increment);
			break;
		case Orientation::down:
			y += static_cast<TyX>(increment);
			break;
		case Orientation::left:
			x -= static_cast<TyX>(increment);
			break;

		default:
			assert(false);
			break;
		}
	}

	inline const char* OrientationToStr(const Orientation orientation) {
		switch (orientation) {
		case Orientation::up:
			return "Up";
		case Orientation::right:
			return "Right";
		case Orientation::down:
			return "Down";
		case Orientation::left:
			return "Left";

		default:
			assert(false);  // Missing switch case
			break;
		}

		return "";
	}
}

#endif // JACTORIO_DATA_PROTOTYPE_TYPE_H
