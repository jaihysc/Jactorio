// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

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

namespace jactorio::game
{
	///
	/// \brief Stores all data for a world
	class World_data
	{
	public:
		World_data() = default;

		~World_data() {
			clear_chunk_data();
		}

		World_data(const World_data& other) = delete;

		World_data(World_data&& other) noexcept
			: game_tick_{other.game_tick_},
			  world_chunks_{std::move(other.world_chunks_)},
			  logic_chunks_{std::move(other.logic_chunks_)},
			  world_gen_seed_{other.world_gen_seed_},
			  world_gen_chunks_{std::move(other.world_gen_chunks_)},
			  deferral_timer{std::move(other.deferral_timer)} {
		}

		World_data& operator=(World_data other) {
			swap(*this, other);
			return *this;
		}

		friend void swap(World_data& lhs, World_data& rhs) noexcept {
			using std::swap;
			swap(lhs.game_tick_, rhs.game_tick_);
			swap(lhs.world_chunks_, rhs.world_chunks_);
			swap(lhs.logic_chunks_, rhs.logic_chunks_);
			swap(lhs.world_gen_seed_, rhs.world_gen_seed_);
			swap(lhs.world_gen_chunks_, rhs.world_gen_chunks_);
			swap(lhs.deferral_timer, rhs.deferral_timer);
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
		using world_chunks_key = uint64_t;

		/// world_chunks_key correlate to a chunk
		std::unordered_map<std::tuple<Chunk::chunk_coord, Chunk::chunk_coord>, Chunk*,
		                   core::hash<std::tuple<Chunk::chunk_coord, Chunk::chunk_coord>>> world_chunks_;

		mutable std::mutex world_chunks_mutex_{};  // Used by methods when accessing world_chunks_

	public:
		using world_coord = int32_t;  // Single world coordinates
		using world_pair = std::pair<world_coord, world_coord>;  // Ordered pair of pocation in the world

		static constexpr uint8_t chunk_width = 32;

		mutable std::mutex world_data_mutex{};  // Held by the thread which is currently operating on a chunk

		///
		/// \brief Converts world coordinate to chunk coordinate
		static Chunk::chunk_coord to_chunk_coord(world_coord world_coord);

		///
		/// \brief Converts world coordinate to struct layer coordinate
		static Chunk_struct_layer::struct_coord to_struct_coord(world_coord world_coord);

		// World access

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
		/// \brief Retrieves a chunk in game world using chunk coordinates
		/// \return nullptr if no chunk exists
		J_NODISCARD Chunk* get_chunk_c(Chunk::chunk_coord chunk_x, Chunk::chunk_coord chunk_y) const;

		///
		/// \brief Retrieves a chunk in game world using chunk coordinates
		/// \return nullptr if no chunk exists
		J_NODISCARD Chunk* get_chunk_c(const Chunk::chunk_pair& chunk_pair) const;


		///
		/// Gets the chunk at the specified world coordinate
		/// \return nullptr if no chunk exists
		J_NODISCARD Chunk* get_chunk(world_coord world_x, world_coord world_y) const;

		///
		/// Gets the chunk at the specified world coordinate
		/// \return nullptr if no chunk exists
		J_NODISCARD Chunk* get_chunk(const world_pair& world_pair) const;

		///
		/// \brief Gets the tile at the specified world coordinate
		/// \return nullptr if no tile exists
		J_NODISCARD Chunk_tile* get_tile(world_coord world_x, world_coord world_y) const;

		///
		/// \brief Gets the tile at the specified world coordinate
		/// \return nullptr if no tile exists
		J_NODISCARD Chunk_tile* get_tile(const world_pair& world_pair) const;


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

		///	
		/// \brief Removes a chunk to be considered for logic updates <br>
		/// \param chunk Logic chunk to remove
		void logic_remove_chunk(Logic_chunk* chunk);

		///
		/// \brief Returns all the chunks which require logic updates
		J_NODISCARD std::map<const Chunk*, Logic_chunk>& logic_get_all_chunks();


		///
		/// \brief Gets logic chunk at Chunk*
		/// \return nullptr if Logic_chunk does not exist
		J_NODISCARD Logic_chunk* logic_get_chunk(const Chunk* chunk);

		///
		/// \brief Gets logic chunk at World coords 
		/// \return nullptr if Logic_chunk or chunk does not exist
		J_NODISCARD Logic_chunk* logic_get_chunk(world_coord world_x, world_coord world_y);

		///
		/// \brief Gets logic chunk at World coords 
		/// \return nullptr if Logic_chunk or chunk does not exist
		J_NODISCARD Logic_chunk* logic_get_chunk(const world_pair& world_pair);


		///
		/// \brief Gets const logic chunk at Chunk* 
		/// \return nullptr if Logic_chunk does not exist
		J_NODISCARD const Logic_chunk* logic_get_chunk(const Chunk* chunk) const;

		/// \brief Gets const logic chunk at World coords 
		/// \return nullptr if Logic_chunk or chunk does not exist
		J_NODISCARD const Logic_chunk* logic_get_chunk(world_coord world_x, world_coord world_y) const;

		///
		/// \brief Gets const logic chunk at World coords 
		/// \return nullptr if Logic_chunk or chunk does not exist
		J_NODISCARD const Logic_chunk* logic_get_chunk(const world_pair& world_pair) const;


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
		void queue_chunk_generation(Chunk::chunk_coord chunk_x, Chunk::chunk_coord chunk_y) const;

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
