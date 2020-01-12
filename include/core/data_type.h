#ifndef CORE_DATA_TYPE_H
#define CORE_DATA_TYPE_H

#include <tuple>

namespace jactorio::core
{
	// Defines standard data pairs for use across this application
	// This is used for better naming compared to std::pair's .first and .second
	
	template <typename T>
	struct Position2
	{
		Position2()
			: x(0), y(0) {
		}

		Position2(T x, T y)
			: x(x), y(y) {
		}
		
		T x;
		T y;
	};

	
	struct Quad_position
	{
		Quad_position() = default;

		Quad_position(const Position2<float> top_left, const Position2<float> bottom_right)
			: top_left(top_left), bottom_right(bottom_right) {
		}
		
		Position2<float> top_left;
		Position2<float> bottom_right;
	};


	// =================================================
	// Defines hash functions for tuples as keys in std::unordered_map
	
	template <typename TT>
	struct hash
	{
		size_t operator()(TT const& tt) const {
			return std::hash<TT>()(tt);
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

	template <typename ... TT>
	struct hash<std::tuple<TT...>>
	{
		size_t operator()(std::tuple<TT...> const& tt) const {
			size_t seed = 0;
			Hash_value_impl<std::tuple<TT...>>::apply(seed, tt);
			return seed;
		}
	};
}

#endif // CORE_DATA_TYPE_H
