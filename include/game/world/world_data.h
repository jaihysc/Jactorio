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

namespace jactorio::game
{
	///
	/// \brief Stores all data for a world
	class WorldData
	{
	public:
		WorldData() = default;

		~WorldData() {
			ClearChunkData();
		}

		WorldData(const WorldData& other) = delete;

		WorldData(WorldData&& other) noexcept
			: gameTick_{other.gameTick_},
			  worldChunks_{std::move(other.worldChunks_)},
			  logicChunks_{std::move(other.logicChunks_)},
			  worldGenSeed_{other.worldGenSeed_},
			  worldGenChunks_{std::move(other.worldGenChunks_)},
			  deferralTimer{std::move(other.deferralTimer)} {
		}

		WorldData& operator=(WorldData other) {
			swap(*this, other);
			return *this;
		}

		friend void swap(WorldData& lhs, WorldData& rhs) noexcept {
			using std::swap;
			swap(lhs.gameTick_, rhs.gameTick_);
			swap(lhs.worldChunks_, rhs.worldChunks_);
			swap(lhs.logicChunks_, rhs.logicChunks_);
			swap(lhs.worldGenSeed_, rhs.worldGenSeed_);
			swap(lhs.worldGenChunks_, rhs.worldGenChunks_);
			swap(lhs.deferralTimer, rhs.deferralTimer);
		}

		// ======================================================================
		// World properties
	private:
		GameTickT gameTick_ = 0;

	public:
		/// \brief Called by the logic loop every update
		void OnTickAdvance();

		///
		/// \brief Number of logic updates since the world was created
		J_NODISCARD GameTickT GameTick() const { return gameTick_; }


		// ======================================================================
		// World chunk
	private:
		// The world is make up of chunks
		// Each chunk contains 32 x 32 tiles
		// 
		// Chunks increment heading right and down
		using WorldChunksKey = uint64_t;

		/// world_chunks_key correlate to a chunk
		std::unordered_map<std::tuple<Chunk::ChunkCoord, Chunk::ChunkCoord>,
		                   Chunk,
		                   core::hash<std::tuple<Chunk::ChunkCoord, Chunk::ChunkCoord>>> worldChunks_;

	public:
		using WorldCoord = int32_t;  // Single world coordinates
		using WorldPair = std::pair<WorldCoord, WorldCoord>;  // Ordered pair of location in the world

		static constexpr uint8_t kChunkWidth = 32;

		mutable std::mutex worldDataMutex{};  // Held by the thread which is currently operating on a chunk

		///
		/// \brief Converts world coordinate to chunk coordinate
		static Chunk::ChunkCoord ToChunkCoord(WorldCoord world_coord);

		// World access

		///
		/// \brief Copy adds a chunk into the game world
		/// Will overwrite existing chunks if they occupy the same position
		/// \param chunk Chunk to be added to the world
		/// \return Pointer to added chunk
		Chunk* AddChunk(const Chunk& chunk);

		///
		/// \brief Adds a chunk into the game world
		/// Will overwrite existing chunks if they occupy the same position
		/// \param args Additional arguments to be provided alongside chunk_x chunk_y to Chunk constructor
		/// \return Pointer to added chunk
		template <typename ... TChunkArgs>
		Chunk* EmplaceChunk(Chunk::ChunkCoord chunk_x, Chunk::ChunkCoord chunk_y,
		                    TChunkArgs ... args) {
			auto conditional = worldChunks_.emplace(std::piecewise_construct,
			                                        std::make_tuple(chunk_x, chunk_y),
			                                        std::make_tuple(chunk_x, chunk_y, args...));
			return &conditional.first->second;
		}

		///
		/// \brief Attempts to delete chunk at chunk_x, chunk_y
		void DeleteChunk(Chunk::ChunkCoord chunk_x, Chunk::ChunkCoord chunk_y);

		///
		/// \brief Erases, frees memory from all stored chunk data + its subsequent contents and logic chunks
		void ClearChunkData();


		///
		/// \brief Retrieves a chunk in game world using chunk coordinates
		/// \return nullptr if no chunk exists
		J_NODISCARD Chunk* GetChunkC(Chunk::ChunkCoord chunk_x, Chunk::ChunkCoord chunk_y);

		///
		/// \brief Retrieves a chunk in game world using chunk coordinates
		/// \return nullptr if no chunk exists
		J_NODISCARD const Chunk* GetChunkC(Chunk::ChunkCoord chunk_x, Chunk::ChunkCoord chunk_y) const;


		///
		/// \brief Retrieves a chunk in game world using chunk coordinates
		/// \return nullptr if no chunk exists
		J_NODISCARD Chunk* GetChunkC(const Chunk::ChunkPair& chunk_pair);

		///
		/// \brief Retrieves a chunk in game world using chunk coordinates
		/// \return nullptr if no chunk exists
		J_NODISCARD const Chunk* GetChunkC(const Chunk::ChunkPair& chunk_pair) const;


		///
		/// Gets the chunk at the specified world coordinate
		/// \return nullptr if no chunk exists
		J_NODISCARD Chunk* GetChunk(WorldCoord world_x, WorldCoord world_y);

		///
		/// Gets the chunk at the specified world coordinate
		/// \return nullptr if no chunk exists
		J_NODISCARD const Chunk* GetChunk(WorldCoord world_x, WorldCoord world_y) const;


		///
		/// Gets the chunk at the specified world coordinate
		/// \return nullptr if no chunk exists
		J_NODISCARD Chunk* GetChunk(const WorldPair& world_pair);

		///
		/// Gets the chunk at the specified world coordinate
		/// \return nullptr if no chunk exists
		J_NODISCARD const Chunk* GetChunk(const WorldPair& world_pair) const;

		// ======================================================================

		///
		/// \brief Gets the tile at the specified world coordinate
		/// \return nullptr if no tile exists
		J_NODISCARD ChunkTile* GetTile(WorldCoord world_x, WorldCoord world_y);

		///
		/// \brief Gets the tile at the specified world coordinate
		/// \return nullptr if no tile exists
		J_NODISCARD const ChunkTile* GetTile(WorldCoord world_x, WorldCoord world_y) const;


		///
		/// \brief Gets the tile at the specified world coordinate
		/// \return nullptr if no tile exists
		J_NODISCARD ChunkTile* GetTile(const WorldPair& world_pair);

		///
		/// \brief Gets the tile at the specified world coordinate
		/// \return nullptr if no tile exists
		J_NODISCARD const ChunkTile* GetTile(const WorldPair& world_pair) const;


		// ==============================================================
		// Logic chunk 
	private:

		std::set<Chunk*> logicChunks_;

	public:
		// Stores chunks which have entities requiring logic updates

		///
		/// \brief Adds a chunk to be considered for logic updates, if the logic chunk already exists at Chunk*,
		/// a reference to the existing one will be returned
		/// \param chunk The chunk this logic chunk is associated with
		void LogicAddChunk(Chunk* chunk);

		///	
		/// \brief Removes a chunk to be considered for logic updates <br>
		/// \param chunk Logic chunk to remove
		void LogicRemoveChunk(Chunk* chunk);

		///
		/// \brief Returns all the chunks which require logic updates
		J_NODISCARD std::set<Chunk*>& LogicGetAllChunks();

		// ======================================================================
		// World generation | Links to game/world/world_generator.cpp
	private:
		int worldGenSeed_ = 1001;

		/// Stores whether or not a chunk is being generated, this gets cleared once all world generation is done
		mutable std::set<Chunk::ChunkPair> worldGenChunks_;
		mutable std::mutex worldGenQueueMutex_;

	public:

		void SetWorldGeneratorSeed(const int seed) { worldGenSeed_ = seed; }
		J_NODISCARD int GetWorldGeneratorSeed() const { return worldGenSeed_; }


		///
		/// \brief Queues a chunk to be generated at specified position
		/// \remark To be called from render thread only
		void QueueChunkGeneration(Chunk::ChunkCoord chunk_x, Chunk::ChunkCoord chunk_y) const;

		///
		/// \brief Takes first in from chunk generation queue and generates chunk
		/// Call once per logic loop tick to generate one chunk only, this keeps performance constant
		/// when generating large amounts of chunks
		void GenChunk(uint8_t amount = 1);


		// ======================================================================
		// Logic Deferral

		DeferralTimer deferralTimer{};
	};
}

#endif //JACTORIO_INCLUDE_GAME_WORLD_WORLD_DATA_H
