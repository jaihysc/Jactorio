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
		using ChunkCoord = int32_t;  // Chunk coordinates
		using ChunkPair = std::pair<ChunkCoord, ChunkCoord>;
		static constexpr uint8_t kChunkWidth = 32;

	private:
		ChunkPair position_;
		// Pointers to the actual tiles since they are static size of 32x32
		ChunkTile* tiles_ = nullptr;

	public:
		///
		/// \brief Default initialization of chunk tiles
		/// \param chunk_x X position of chunk
		/// \param chunk_y Y position of chunk
		Chunk(ChunkCoord chunk_x, ChunkCoord chunk_y);
		///
		/// \param chunk_x X position of chunk
		/// \param chunk_y Y position of chunk
		/// \param tiles Array of size 32 * 32 (1024) tiles <br>
	    ///			Do not delete the provided pointer, it will be automatically
		///			freed when Chunk is destructed
		Chunk(ChunkCoord chunk_x, ChunkCoord chunk_y, ChunkTile* tiles);

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

		J_NODISCARD std::pair<int, int> GetPosition() const { return position_; }

		J_NODISCARD ChunkTile* Tiles() const {
			return tiles_;
		}

		// Objects - Rendered without being fixed to a tile position
		enum class ObjectLayer
		{
			debug_overlay = 0,  // data::Sprite
			count_
		};

		static constexpr int kObjectLayerCount = static_cast<int>(ObjectLayer::count_);

		std::vector<ChunkObjectLayer> objects[kObjectLayerCount];

		std::vector<ChunkObjectLayer>& GetObject(ObjectLayer layer) {
			return objects[static_cast<int>(layer)];
		}
	};
}

#endif //JACTORIO_INCLUDE_GAME_WORLD_CHUNK_H
