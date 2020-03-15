// 
// world_data.h
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 10/22/2019
// Last modified: 03/12/2020
// 

#ifndef JACTORIO_INCLUDE_GAME_WORLD_WORLD_DATA_H
#define JACTORIO_INCLUDE_GAME_WORLD_WORLD_DATA_H
#pragma once

#include "game/world/chunk.h"
#include "game/world/logic_chunk.h"
#include "core/data_type.h"

#include <set>
#include <unordered_map>

// Manages the game world, the tiles and the entities on it
// Handles saving and loading the world
namespace jactorio::game
{
	class World_data
	{
	public:
		World_data() = default;

		World_data(const World_data& other) = delete;
		World_data(World_data&& other) noexcept = delete;

		World_data& operator=(const World_data& other) = delete;
		World_data& operator=(World_data&& other) noexcept = delete;

		~World_data() {
			clear_chunk_data();
		}

	private:
		// The world is make up of chunks
		// Each chunk contains 32 x 32 tiles
		// 
		// Chunks increment heading right and down
		using world_chunks_key = unsigned long long;

		/// world_chunks_key correlate to a chunk
		std::unordered_map<std::tuple<int, int>, Chunk*,
		                   core::hash<std::tuple<int, int>>> world_chunks_;

		std::map<Chunk*, Logic_chunk> logic_chunks_;

		std::mutex mutex_{};

	public:

		///
		/// Adds a chunk into the game world <br>
		/// Will overwrite existing chunks if they occupy the same position, the overriden chunk's
		/// destructor will be called <br>
		/// Do NOT delete the provided chunk pointer, it will be automatically deleted
		/// @param chunk Chunk to be added to the world
		/// @return Pointer to added chunk
		Chunk* add_chunk(Chunk* chunk);

		///
		/// Retrieves a chunk in game world
		/// @param chunk_x X position of CHUNK
		/// @param chunk_y Y position of CHUNK
		Chunk* get_chunk(int chunk_x, int chunk_y);

		///
		/// Erases, frees memory from all stored chunk data + its subsequent contents and logic chunks
		void clear_chunk_data();


		// ==============================================================
		// Chunk tiles

		///
		/// Gets the tile at the specified world coordinate
		/// @return nullptr if no tile exists
		Chunk_tile* get_tile_world_coords(int world_x, int world_y);

		///
		/// Gets the chunk at the specified world coordinate
		/// @return nullptr if no chunk exists
		Chunk* get_chunk_world_coords(int world_x, int world_y);


		// ==============================================================
		// World logic

		// Stores chunks which have entities requiring logic updates

		///
		/// Adds a chunk to be considered for logic updates
		/// @param chunk The chunk this logic chunk is associated with
		/// @return Reference to the added chunk
		Logic_chunk& logic_add_chunk(Chunk* chunk);

		 //
		 // Removes a chunk to be considered for logic updates <br>
		 // O(n) time complexity
		 // @param chunk Logic chunk to remove
		 // void logic_remove_chunk(Logic_chunk* chunk);

		///
		/// Returns all the chunks which require logic updates
		std::map<Chunk*, Logic_chunk>& logic_get_all_chunks();


		// ======================================================================
		// World generation | Links to game/world/world_generator.cpp
	private:
		int world_gen_seed_ = 1001;

		/// Stores whether or not a chunk is being generated, this gets cleared once all world generation is done
		std::set<std::tuple<int, int>> world_gen_chunks_;
		std::mutex world_gen_queue_mutex_;

	public:
		
		void set_world_generator_seed(const int seed) { world_gen_seed_ = seed; }
		J_NODISCARD int get_world_generator_seed() const { return world_gen_seed_; }


		///
		/// \brief Queues a chunk to be generated at specified position
		void queue_chunk_generation(int chunk_x, int chunk_y);

		///
		/// \brief Takes first in from chunk generation queue and generates chunk
		/// Call once per logic loop tick to generate one chunk only, this keeps performance constant
		/// when generating large amounts of chunks
		void gen_chunk(uint8_t amount = 1);
	};
}

#endif //JACTORIO_INCLUDE_GAME_WORLD_WORLD_DATA_H
