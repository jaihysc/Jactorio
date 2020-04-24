// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 01/12/2020

#ifndef JACTORIO_INCLUDE_CORE_DATA_TYPE_H
#define JACTORIO_INCLUDE_CORE_DATA_TYPE_H
#pragma once

#include <decimal.h>

#include <tuple>

// Data types of the various components within Jactorio
namespace jactorio
{
	constexpr int transport_line_decimal_place = 3;
	using transport_line_offset = dec::decimal<transport_line_decimal_place>;

	using game_tick_t = uint64_t;
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


	struct Quad_position
	{
		Quad_position() = default;

		Quad_position(const Position2<float> top_left, const Position2<float> bottom_right)
			: top_left(top_left), bottom_right(bottom_right) {
		}

		Position2<float> top_left;
		Position2<float> bottom_right;

		Quad_position operator+(const Quad_position& other) const {
			Quad_position result;
			result.top_left = this->top_left + other.top_left;
			result.bottom_right = this->bottom_right + other.bottom_right;

			return result;
		}

		void operator+=(const Quad_position& other) {
			this->top_left += other.top_left;
			this->bottom_right += other.bottom_right;
		}


		Quad_position operator-(const Quad_position& other) const {
			Quad_position result;
			result.top_left = this->top_left - other.top_left;
			result.bottom_right = this->bottom_right - other.bottom_right;

			return result;
		}

		void operator-=(const Quad_position& other) {
			this->top_left -= other.top_left;
			this->bottom_right -= other.bottom_right;
		}


		Quad_position operator*(const Quad_position& other) const {
			Quad_position result;
			result.top_left = this->top_left * other.top_left;
			result.bottom_right = this->bottom_right * other.bottom_right;

			return result;
		}

		void operator*=(const Quad_position& other) {
			this->top_left *= other.top_left;
			this->bottom_right *= other.bottom_right;
		}


		Quad_position operator/(const Quad_position& other) const {
			Quad_position result;
			result.top_left = this->top_left / other.top_left;
			result.bottom_right = this->bottom_right / other.bottom_right;

			return result;
		}

		void operator/=(const Quad_position& other) {
			this->top_left /= other.top_left;
			this->bottom_right /= other.bottom_right;
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
