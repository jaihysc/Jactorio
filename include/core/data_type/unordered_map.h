#ifndef CORE_DATA_TYPE_UNORDERED_MAP_H
#define CORE_DATA_TYPE_UNORDERED_MAP_H

#include <tuple>

// Defines hash functions for tuples as keys in std::unordered_map

namespace jactorio::core
{
	// TODO this hash function may be flawed, find a new one?
	
	template <typename TT>
	struct hash
	{
		size_t operator()(TT const& tt) const {
			return std::hash<TT>()(tt);
		}
	};
	
	template <class T>
	void hash_combine(std::size_t& seed, T const& v) {
		seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
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

#endif // CORE_DATA_TYPE_UNORDERED_MAP_H
