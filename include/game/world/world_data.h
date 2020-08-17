// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_WORLD_WORLD_DATA_H
#define JACTORIO_INCLUDE_GAME_WORLD_WORLD_DATA_H
#pragma once

#include <set>
#include <unordered_map>
#include <utility>

#include "jactorio.h"
#include "core/data_type.h"
#include "game/world/chunk.h"
#include "game/world/update_dispatcher.h"

namespace jactorio::data
{
	enum class UpdateType;

	class IDeferred;
	class IUpdateListener;
}

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
		WorldData(WorldData&& other)      = delete;

		// ======================================================================
		// World chunk
		static constexpr uint8_t kChunkWidth = 32;

		mutable std::mutex worldDataMutex{};  // Held by the thread which is currently operating on a chunk

		///
		/// \brief Converts world coordinate to chunk coordinate
		static ChunkCoordAxis ToChunkCoord(WorldCoordAxis world_coord);

		///
		/// \brief Converts world coordinate to overlay element coordinates
		static OverlayOffsetAxis ToOverlayCoord(WorldCoordAxis world_coord);


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
		Chunk* EmplaceChunk(ChunkCoordAxis chunk_x, ChunkCoordAxis chunk_y,
		                    TChunkArgs ... args) {
			auto conditional = worldChunks_.emplace(std::piecewise_construct,
			                                        std::make_tuple(chunk_x, chunk_y),
			                                        std::make_tuple(chunk_x, chunk_y, args...));
			return &conditional.first->second;
		}

		///
		/// \brief Attempts to delete chunk at chunk_x, chunk_y
		void DeleteChunk(ChunkCoordAxis chunk_x, ChunkCoordAxis chunk_y);

		///
		/// \brief Erases, frees memory from all stored chunk data + its subsequent contents and logic chunks
		void ClearChunkData();


		///
		/// \brief Retrieves a chunk in game world using chunk coordinates
		/// \return nullptr if no chunk exists
		J_NODISCARD Chunk* GetChunkC(ChunkCoordAxis chunk_x, ChunkCoordAxis chunk_y);

		///
		/// \brief Retrieves a chunk in game world using chunk coordinates
		/// \return nullptr if no chunk exists
		J_NODISCARD const Chunk* GetChunkC(ChunkCoordAxis chunk_x, ChunkCoordAxis chunk_y) const;


		///
		/// \brief Retrieves a chunk in game world using chunk coordinates
		/// \return nullptr if no chunk exists
		J_NODISCARD Chunk* GetChunkC(const ChunkCoord& chunk_pair);

		///
		/// \brief Retrieves a chunk in game world using chunk coordinates
		/// \return nullptr if no chunk exists
		J_NODISCARD const Chunk* GetChunkC(const ChunkCoord& chunk_pair) const;


		///
		/// Gets the chunk at the specified world coordinate
		/// \return nullptr if no chunk exists
		J_NODISCARD Chunk* GetChunkW(WorldCoordAxis world_x, WorldCoordAxis world_y);

		///
		/// Gets the chunk at the specified world coordinate
		/// \return nullptr if no chunk exists
		J_NODISCARD const Chunk* GetChunkW(WorldCoordAxis world_x, WorldCoordAxis world_y) const;


		///
		/// Gets the chunk at the specified world coordinate
		/// \return nullptr if no chunk exists
		J_NODISCARD Chunk* GetChunkW(const WorldCoord& world_pair);

		///
		/// Gets the chunk at the specified world coordinate
		/// \return nullptr if no chunk exists
		J_NODISCARD const Chunk* GetChunkW(const WorldCoord& world_pair) const;

		// ======================================================================

		///
		/// \brief Gets the tile at the specified world coordinate
		/// \return nullptr if no tile exists
		J_NODISCARD ChunkTile* GetTile(WorldCoordAxis world_x, WorldCoordAxis world_y);

		///
		/// \brief Gets the tile at the specified world coordinate
		/// \return nullptr if no tile exists
		J_NODISCARD const ChunkTile* GetTile(WorldCoordAxis world_x, WorldCoordAxis world_y) const;


		///
		/// \brief Gets the tile at the specified world coordinate
		/// \return nullptr if no tile exists
		J_NODISCARD ChunkTile* GetTile(const WorldCoord& world_pair);

		///
		/// \brief Gets the tile at the specified world coordinate
		/// \return nullptr if no tile exists
		J_NODISCARD const ChunkTile* GetTile(const WorldCoord& world_pair) const;


		// ==============================================================
		// Logic chunk 

		///
		/// \brief Adds a layer at coordinates to be considered for logic updates
		void LogicRegister(Chunk::LogicGroup group, const WorldCoord& world_pair, ChunkTile::ChunkLayer layer);

		///
		/// \brief Removes a layer at coordinates to be considered for logic updates
		/// w/ custom comparison func to remove
		void LogicRemove(Chunk::LogicGroup group, const WorldCoord& world_pair,
		                 const std::function<bool(ChunkTileLayer*)>& pred);

		///
		/// \brief Removes a layer at coordinates to be considered for logic updates
		void LogicRemove(Chunk::LogicGroup group, const WorldCoord& world_pair, ChunkTile::ChunkLayer layer);


		///
		/// \brief Adds a chunk to be considered for logic updates, if the logic chunk already exists at Chunk*,
		/// a reference to the existing one will be returned
		/// \param chunk The chunk this logic chunk is associated with
		void LogicAddChunk(Chunk* chunk);

		///
		/// \brief Returns all the chunks which require logic updates
		J_NODISCARD std::set<Chunk*>& LogicGetChunks();

		// ======================================================================
		// World generation

		void SetWorldGeneratorSeed(const int seed) { worldGenSeed_ = seed; }
		J_NODISCARD int GetWorldGeneratorSeed() const { return worldGenSeed_; }


		///
		/// \brief Queues a chunk to be generated at specified position
		/// \remark To be called from render thread only
		void QueueChunkGeneration(ChunkCoordAxis chunk_x, ChunkCoordAxis chunk_y) const;

		///
		/// \brief Takes first in from chunk generation queue and generates chunk
		/// Call once per logic loop tick to generate one chunk only, this keeps performance constant
		/// when generating large amounts of chunks
		void GenChunk(const data::PrototypeManager& data_manager, uint8_t amount = 1);


		// ======================================================================


		CEREAL_SERIALIZE(archive) {
			archive(updateDispatcher, worldChunks_, worldGenSeed_); //, m.worldChunks_, m.logicChunks_);
		}

		UpdateDispatcher updateDispatcher;

	private:
		using ChunkKey = std::tuple<ChunkCoordAxis, ChunkCoordAxis>;
		using ChunkHasher = core::hash<ChunkKey>;

		/// Chunks increment heading right and down
		std::unordered_map<ChunkKey, Chunk, ChunkHasher> worldChunks_;
		std::set<Chunk*> logicChunks_;


		mutable std::mutex worldGenQueueMutex_;

		int worldGenSeed_ = 1001;
		/// Stores whether or not a chunk is being generated, this gets cleared once all world generation is done
		mutable std::set<ChunkKey> worldGenChunks_;
	};
}

#endif //JACTORIO_INCLUDE_GAME_WORLD_WORLD_DATA_H
