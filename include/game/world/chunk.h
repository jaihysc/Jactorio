// 
// chunk.h
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 10/29/2019
// Last modified: 03/14/2020
// 

#ifndef JACTORIO_INCLUDE_GAME_WORLD_CHUNK_H
#define JACTORIO_INCLUDE_GAME_WORLD_CHUNK_H
#pragma once

#include "jactorio.h"

#include <utility>
#include <vector>

#include "game/world/chunk_tile.h"
#include "game/world/chunk_object_layer.h"

namespace jactorio::game
{
	/**
	 * A chunk within the game <br>
	 * Made up of tiles and objects:
	 *		tiles: Has 32 x 32, fixed grid location
	 *		objects: Has no set amount, can exist anywhere on chunk
	 */
	class Chunk
	{
		std::pair<int, int> position_;
		// Pointers to the actual tiles since they are static size of 32x32
		Chunk_tile* tiles_ = nullptr;

	public:
		/**
		 * @param x X position of chunk
		 * @param y Y position of chunk
		 * @param tiles Array of size 32 * 32 (1024) tiles <br>
		 *			provide nullptr to use default tile initialization <br>
		 *			Do not delete the provided pointer, it will be automatically
		 *			freed when Chunk is destructed
		 */
		Chunk(int x, int y, Chunk_tile* tiles);
		~Chunk();

		Chunk(const Chunk& other);
		Chunk(Chunk&& other) noexcept;

		Chunk& operator=(const Chunk& other);

		Chunk& operator=(Chunk&& other) noexcept;

		// Tiles

		J_NODISCARD std::pair<int, int> get_position() const;

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
