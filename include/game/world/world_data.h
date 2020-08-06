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
	private:
		// The world is make up of chunks
		// Each chunk contains 32 x 32 tiles
		// 
		// Chunks increment heading right and down
		using WorldChunksKey = uint64_t;

		/// world_chunks_key correlate to a chunk
		std::unordered_map<std::tuple<ChunkCoordAxis, ChunkCoordAxis>,
		                   Chunk,
		                   core::hash<std::tuple<ChunkCoordAxis, ChunkCoordAxis>>> worldChunks_;

	public:
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
	private:

		std::set<Chunk*> logicChunks_;

	public:
		// Stores chunks which have entities requiring logic updates

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
	private:
		int worldGenSeed_ = 1001;

		/// Stores whether or not a chunk is being generated, this gets cleared once all world generation is done
		mutable std::set<std::pair<ChunkCoordAxis, ChunkCoordAxis>> worldGenChunks_;
		mutable std::mutex worldGenQueueMutex_;

	public:

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

		///
		/// \brief Calls callbacks for tile updates
		class UpdateDispatcher
		{
			using CallbackT = const data::IUpdateListener*;
			using CollectionT = std::vector<std::pair<WorldCoord, CallbackT>>;

			using ContainerKeyT = std::tuple<WorldCoordAxis, WorldCoordAxis>;
			using ContainerT = std::unordered_map<ContainerKeyT, CollectionT, core::hash<ContainerKeyT>>;

			ContainerT container_;

			struct DebugInfo;

		public:
			/// Current world coord, Registered world coord 
			using ListenerEntry = std::pair<WorldCoord, WorldCoord>;

			explicit UpdateDispatcher(WorldData& world_data)
				: worldData_(world_data) {
			}

			///
			/// \brief Registers proto_listener callback when target coords is updated, providing current coords
			ListenerEntry Register(WorldCoordAxis current_world_x, WorldCoordAxis current_world_y,
			                       WorldCoordAxis target_world_x, WorldCoordAxis target_world_y,
			                       const data::IUpdateListener& proto_listener);

			///
			/// \brief Registers proto_listener callback when target coords is updated, providing current coords
			ListenerEntry Register(const WorldCoord& current_coords, const WorldCoord& target_coords,
			                       const data::IUpdateListener& proto_listener);

			///
			/// \brief Unregisters entry
			/// \return true if succeeded, false if failed
			bool Unregister(const ListenerEntry& entry);

			void Dispatch(WorldCoordAxis world_x, WorldCoordAxis world_y, data::UpdateType type);
			void Dispatch(const WorldCoord& world_pair, data::UpdateType type);

			J_NODISCARD DebugInfo GetDebugInfo() const noexcept;

		private:
			WorldData& worldData_;

			struct DebugInfo
			{
				const ContainerT& storedEntries;
			};

		} updateDispatcher{*this};
	};
}

#endif //JACTORIO_INCLUDE_GAME_WORLD_WORLD_DATA_H
