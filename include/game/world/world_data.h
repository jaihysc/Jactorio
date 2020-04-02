// 
// world_data.h
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 03/31/2020
// Last modified: 04/02/2020
// 

#ifndef JACTORIO_INCLUDE_GAME_WORLD_WORLD_DATA_H
#define JACTORIO_INCLUDE_GAME_WORLD_WORLD_DATA_H
#pragma once

#include <set>
#include <unordered_map>

#include "jactorio.h"
#include "core/data_type.h"
#include "game/world/chunk.h"
#include "game/world/deferral_timer.h"
#include "game/world/logic_chunk.h"

// Manages the game world, the tiles and the entities on it
// Handles saving and loading the world
namespace jactorio::game
{
	///
	/// \brief Stores all data for a world
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


		// ======================================================================
		// World properties
	private:
		game_tick_t game_tick_ = 0;

	public:
		/// \brief Called by the logic loop every update
		void on_tick_advance();

		///
		/// \brief Number of logic updates since the world was created
		J_NODISCARD game_tick_t game_tick() const { return game_tick_; }


		// ======================================================================
		// World chunk
	private:
		// The world is make up of chunks
		// Each chunk contains 32 x 32 tiles
		// 
		// Chunks increment heading right and down
		using world_chunks_key = unsigned long long;

		/// world_chunks_key correlate to a chunk
		std::unordered_map<std::tuple<int, int>, Chunk*,
		                   core::hash<std::tuple<int, int>>> world_chunks_;

		mutable std::mutex world_chunks_mutex_{};  // Used by methods when accessing world_chunks_

	public:
		using world_coord = int32_t;  // World coordinates
		using chunk_coord = int32_t;  // Chunk coordinates

		mutable std::mutex world_data_mutex{};  // Held by the thread which is currently operating on a chunk


		///
		/// \brief Adds a chunk into the game world
		/// Will overwrite existing chunks if they occupy the same position, the overriden chunk will be deleted
		/// \remark Do NOT delete the provided chunk pointer, it will be automatically deleted
		/// \param chunk Chunk to be added to the world
		/// \return Pointer to added chunk
		Chunk* add_chunk(Chunk* chunk);

		///
		/// \brief Erases, frees memory from all stored chunk data + its subsequent contents and logic chunks
		void clear_chunk_data();


		///
		/// \brief Retrieves a chunk in game world
		/// \return nullptr if no chunk exists
		J_NODISCARD Chunk* get_chunk(chunk_coord chunk_x, chunk_coord chunk_y);

		///
		/// \brief Retrieves a read only chunk in game world
		/// \return nullptr if no chunk exists
		J_NODISCARD const Chunk* get_chunk_read_only(chunk_coord chunk_x, chunk_coord chunk_y) const;

		///
		/// Gets the chunk at the specified world coordinate
		/// \return nullptr if no chunk exists
		J_NODISCARD Chunk* get_chunk_world_coords(world_coord world_x, world_coord world_y);


		///
		/// \brief Gets the tile at the specified world coordinate
		/// \return nullptr if no tile exists
		J_NODISCARD Chunk_tile* get_tile_world_coords(world_coord world_x, world_coord world_y);


		// ==============================================================
		// Logic chunk 
	private:

		std::map<const Chunk*, Logic_chunk> logic_chunks_;

	public:
		// Stores chunks which have entities requiring logic updates

		///
		/// \brief Adds a chunk to be considered for logic updates, if the logic chunk already exists at Chunk*,
		/// a reference to the existing one will be returned
		/// \param chunk The chunk this logic chunk is associated with
		/// \return Reference to the added chunk
		Logic_chunk& logic_add_chunk(Chunk* chunk);

		//
		// Removes a chunk to be considered for logic updates <br>
		// O(n) time complexity
		// @param chunk Logic chunk to remove
		// void logic_remove_chunk(Logic_chunk* chunk);

		///
		/// \brief Returns all the chunks which require logic updates
		J_NODISCARD std::map<const Chunk*, Logic_chunk>& logic_get_all_chunks();

		///
		/// \brief Gets logic chunk at Chunk*
		/// \return nullptr if Logic_chunk does not exist
		J_NODISCARD Logic_chunk* logic_get_chunk(const Chunk* chunk);

		///
		/// \brief Gets read only logic chunk at Chunk* 
		/// \return nullptr if Logic_chunk does not exist
		J_NODISCARD const Logic_chunk* logic_get_chunk_read_only(const Chunk* chunk) const;


		// ======================================================================
		// World generation | Links to game/world/world_generator.cpp
	private:
		int world_gen_seed_ = 1001;

		/// Stores whether or not a chunk is being generated, this gets cleared once all world generation is done
		mutable std::set<std::tuple<int, int>> world_gen_chunks_;
		mutable std::mutex world_gen_queue_mutex_;

	public:

		void set_world_generator_seed(const int seed) { world_gen_seed_ = seed; }
		J_NODISCARD int get_world_generator_seed() const { return world_gen_seed_; }


		///
		/// \brief Queues a chunk to be generated at specified position
		/// \remark To be called from render thread only
		void queue_chunk_generation(chunk_coord chunk_x, chunk_coord chunk_y) const;

		///
		/// \brief Takes first in from chunk generation queue and generates chunk
		/// Call once per logic loop tick to generate one chunk only, this keeps performance constant
		/// when generating large amounts of chunks
		void gen_chunk(uint8_t amount = 1);


		// ======================================================================
		// Logic Deferral

		Deferral_timer deferral_timer{};
	};
}

#endif //JACTORIO_INCLUDE_GAME_WORLD_WORLD_DATA_H
