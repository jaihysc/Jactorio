// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_CORE_DATA_TYPE_H
#define JACTORIO_INCLUDE_CORE_DATA_TYPE_H
#pragma once

#include <tuple>

#include "jactorio.h"

// Data types of the various components within Jactorio
namespace jactorio
{
	using GameTickT = uint64_t;
	constexpr int kGameHertz = 60;  // 60 updates per second
}

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

		Position2 operator+(const Position2& other) const {
			Position2 result;
			result.x = this->x + other.x;
			result.y = this->y + other.y;

			return result;
		}

		void operator+=(const Position2& other) {
			this->x += other.x;
			this->y += other.y;
		}


		Position2 operator-(const Position2& other) const {
			Position2 result;
			result.x = this->x - other.x;
			result.y = this->y - other.y;

			return result;
		}

		void operator-=(const Position2& other) {
			this->x -= other.x;
			this->y -= other.y;
		}


		Position2 operator*(const Position2& other) const {
			Position2 result;
			result.x = this->x * other.x;
			result.y = this->y * other.y;

			return result;
		}

		void operator*=(const Position2& other) {
			this->x *= other.x;
			this->y *= other.y;
		}


		Position2 operator/(const Position2& other) const {
			Position2 result;
			result.x = this->x / other.x;
			result.y = this->y / other.y;

			return result;
		}

		void operator/=(const Position2& other) {
			this->x /= other.x;
			this->y /= other.y;
		}
	};


	struct QuadPosition
	{
		QuadPosition() = default;

		QuadPosition(const Position2<float> top_left, const Position2<float> bottom_right)
			: topLeft(top_left), bottomRight(bottom_right) {
		}

		Position2<float> topLeft;
		Position2<float> bottomRight;

		QuadPosition operator+(const QuadPosition& other) const {
			QuadPosition result;
			result.topLeft     = this->topLeft + other.topLeft;
			result.bottomRight = this->bottomRight + other.bottomRight;

			return result;
		}

		void operator+=(const QuadPosition& other) {
			this->topLeft += other.topLeft;
			this->bottomRight += other.bottomRight;
		}


		QuadPosition operator-(const QuadPosition& other) const {
			QuadPosition result;
			result.topLeft     = this->topLeft - other.topLeft;
			result.bottomRight = this->bottomRight - other.bottomRight;

			return result;
		}

		void operator-=(const QuadPosition& other) {
			this->topLeft -= other.topLeft;
			this->bottomRight -= other.bottomRight;
		}


		QuadPosition operator*(const QuadPosition& other) const {
			QuadPosition result;
			result.topLeft     = this->topLeft * other.topLeft;
			result.bottomRight = this->bottomRight * other.bottomRight;

			return result;
		}

		void operator*=(const QuadPosition& other) {
			this->topLeft *= other.topLeft;
			this->bottomRight *= other.bottomRight;
		}


		QuadPosition operator/(const QuadPosition& other) const {
			QuadPosition result;
			result.topLeft     = this->topLeft / other.topLeft;
			result.bottomRight = this->bottomRight / other.bottomRight;

			return result;
		}

		void operator/=(const QuadPosition& other) {
			this->topLeft /= other.topLeft;
			this->bottomRight /= other.bottomRight;
		}
	};


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
