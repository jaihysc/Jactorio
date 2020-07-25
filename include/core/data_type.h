// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_CORE_DATA_TYPE_H
#define JACTORIO_INCLUDE_CORE_DATA_TYPE_H
#pragma once

#include <tuple>
#include <unordered_map>

namespace jactorio::core
{
	template <typename TVal>
	struct Position2;
	
	template <typename TPosition>
	struct QuadPosition;
}

namespace jactorio
{
	// Data types of the various components within Jactorio
	// Defined here to solve circular includes

	using GameTickT = uint64_t;
	constexpr int kGameHertz = 60;  // 60 updates per second


	using WorldCoordAxis = int;
	using WorldCoord = core::Position2<WorldCoordAxis>;

	
	using UvPositionT = core::QuadPosition<core::Position2<float>>;
	/// Internal id of sprite to uv coordinates
	using SpriteUvCoordsT = std::unordered_map<unsigned int, UvPositionT>;
}

namespace jactorio::core
{
	// Defines standard data pairs for use across this application
	// This is used for better naming compared to std::pair's .first and .second

	// =================================================
	// Defines hash functions for tuples as keys in std::unordered_map

	template <typename Tt>
	struct hash
	{
		size_t operator()(Tt const& tt) const {
			return std::hash<Tt>()(tt);
		}
	};

	template <class T>
	void hash_combine(std::size_t& seed, T const& v) {
		seed ^= hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	template <class Tuple, size_t Index = std::tuple_size<Tuple>::value - 1>
	struct Hash_value_impl
	{
		static void apply(size_t& seed, Tuple const& tuple) {
			Hash_value_impl<Tuple, Index - 1>::apply(seed, tuple);
			hash_combine(seed, std::get<Index>(tuple));
		}
	};

	template <class Tuple>
	struct Hash_value_impl<Tuple, 0>
	{
		static void apply(size_t& seed, Tuple const& tuple) {
			hash_combine(seed, std::get<0>(tuple));
		}
	};

	template <typename ... Tt>
	struct hash<std::tuple<Tt...>>
	{
		size_t operator()(std::tuple<Tt...> const& tt) const {
			size_t seed = 0;
			Hash_value_impl<std::tuple<Tt...>>::apply(seed, tt);
			return seed;
		}
	};
}

#endif //JACTORIO_INCLUDE_CORE_DATA_TYPE_H
