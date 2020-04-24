// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/29/2019

#ifndef JACTORIO_INCLUDE_GAME_WORLD_CHUNK_H
#define JACTORIO_INCLUDE_GAME_WORLD_CHUNK_H
#pragma once

#include "jactorio.h"

#include <utility>
#include <vector>

#include "game/world/chunk_object_layer.h"
#include "game/world/chunk_tile.h"

namespace jactorio::game
{
	///
	/// \brief A chunk within the game <br>
	/// Made up of tiles and objects:
	///		tiles: Has 32 x 32, fixed grid location
	///		objects: Has no set amount, can exist anywhere on chunk
	class Chunk
	{
	public:
		using chunk_coord = int32_t;  // Chunk coordinates
		static constexpr uint8_t chunk_width = 32;

	private:
		std::pair<chunk_coord, chunk_coord> position_;
		// Pointers to the actual tiles since they are static size of 32x32
		Chunk_tile* tiles_ = nullptr;

	public:
		///
		/// \brief Default initialization of chunk tiles
		/// \param chunk_x X position of chunk
		/// \param chunk_y Y position of chunk
		Chunk(chunk_coord chunk_x, chunk_coord chunk_y);
		///
		/// \param chunk_x X position of chunk
		/// \param chunk_y Y position of chunk
		/// \param tiles Array of size 32 * 32 (1024) tiles <br>
	    ///			Do not delete the provided pointer, it will be automatically
		///			freed when Chunk is destructed
		Chunk(chunk_coord chunk_x, chunk_coord chunk_y, Chunk_tile* tiles);

		~Chunk();

		Chunk(const Chunk& other);
		Chunk(Chunk&& other) noexcept;

		Chunk& operator=(Chunk other);

		friend void swap(Chunk& lhs, Chunk& rhs) noexcept {
			using std::swap;
			swap(lhs.position_, rhs.position_);
			swap(lhs.tiles_, rhs.tiles_);
		}

		// Tiles

		J_NODISCARD std::pair<int, int> get_position() const { return position_; }

		J_NODISCARD Chunk_tile* tiles_ptr() const {
			return tiles_;
		}

		// Objects - Rendered without being fixed to a tile position
		enum class objectLayer
		{
			tree = 0,
			debug_overlay,  // data::Sprite
			count_
		};

		static constexpr int object_layer_count = static_cast<int>(objectLayer::count_);

		std::vector<Chunk_object_layer> objects[object_layer_count];

		std::vector<Chunk_object_layer>& get_object(objectLayer layer) {
			return objects[static_cast<int>(layer)];
		}
	};
}

#endif //JACTORIO_INCLUDE_GAME_WORLD_CHUNK_H
