// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_CORE_COORDINATE_TUPLE_H
#define JACTORIO_CORE_COORDINATE_TUPLE_H
#pragma once

namespace jactorio::core
{
	template <typename TVal>
	struct Position1
	{
		static_assert(std::is_trivial<TVal>::value);

		using ValueT = TVal;

		Position1()
			: x(0) {
		}

		explicit Position1(TVal x)
			: x(x) {
		}

		TVal x;
	};

	template <typename TVal>
	struct Position2 : Position1<TVal>
	{
		using ValueT = TVal;

		Position2()
			: Position1<TVal>(), y(0) {
		}

		Position2(const Position1<TVal>& x, TVal y)
			: Position1<TVal>(x), y(y) {
		}

		Position2(TVal x, TVal y)
			: Position1<TVal>(x), y(y) {
		}

		TVal y;
	};

	template <typename TVal>
	struct Position3 : Position2<TVal>
	{
		using ValueT = TVal;

		Position3()
			: Position2<TVal>(), z(0) {
		}

		Position3(const Position2<TVal>& xy, TVal z)
			: Position2<TVal>(xy), z(z) {
		}

		Position3(TVal x, TVal y, TVal z)
			: Position2<TVal>(x, y), z(z) {
		}

		TVal z;
	};


	template <typename TPosition>
	struct QuadPosition
	{
		static_assert(std::is_base_of<Position1<typename TPosition::ValueT>, TPosition>::value);

		using PositionT = TPosition;

		QuadPosition() = default;

		QuadPosition(const TPosition top_left, const TPosition bottom_right)
			: topLeft(top_left), bottomRight(bottom_right) {
		}

		TPosition topLeft;
		TPosition bottomRight;
	};
}

#endif // JACTORIO_CORE_COORDINATE_TUPLE_H
