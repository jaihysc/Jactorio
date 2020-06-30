// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

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
		/// \brief Adds a layer at coordinates to be considered for logic updates
		void LogicRegister(Chunk::LogicGroup group, const WorldPair& world_pair, ChunkTile::ChunkLayer layer);

		///
		/// \brief Removes a layer at coordinates to be considered for logic updates
		/// w/ custom comparison func to remove
		void LogicRemove(Chunk::LogicGroup group, const WorldPair& world_pair,
		                 const std::function<bool(ChunkTileLayer*)>& pred);

		///
		/// \brief Removes a layer at coordinates to be considered for logic updates
		void LogicRemove(Chunk::LogicGroup group, const WorldPair& world_pair, ChunkTile::ChunkLayer layer);


		///
		/// \brief Adds a chunk to be considered for logic updates, if the logic chunk already exists at Chunk*,
		/// a reference to the existing one will be returned
		/// \param chunk The chunk this logic chunk is associated with
		void LogicAddChunk(Chunk* chunk);

		///
		/// \brief Returns all the chunks which require logic updates
		J_NODISCARD std::set<Chunk*>& LogicGetChunks();

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
		void GenChunk(const data::DataManager& data_manager, uint8_t amount = 1);


		// ======================================================================

		///
		/// \brief Manages deferrals, prototypes inheriting 'Deferred'
		class DeferralTimer
		{
			/// \brief vector of callbacks at game tick
			std::unordered_map<GameTickT,
			                   std::vector<
				                   std::pair<std::reference_wrapper<const data::IDeferred>, data::UniqueDataBase*>
			                   >> callbacks_;

			/// \brief 0 indicates invalid callback
			using CallbackIndex = decltype(callbacks_.size());

		public:
			explicit DeferralTimer(WorldData& world_data)
				: worldData_(world_data) {
			}

			/// \brief Information about the registered deferral for removing
			///
			/// .second value of 0 indicates invalid callback
			using DeferralEntry = std::pair<GameTickT, CallbackIndex>;

			///
			/// \brief Calls all deferred callbacks for the current game tick
			/// \param game_tick Current game tick
			void DeferralUpdate(GameTickT game_tick);

			///
			/// \brief Registers callback which will be called upon reaching the specified game tick
			/// \param deferred Implements virtual function on_defer_time_elapsed
			/// \param due_game_tick Game tick where the callback will be called
			/// \return Index of registered callback, use this to remove the callback later
			DeferralEntry RegisterAtTick(const data::IDeferred& deferred, data::UniqueDataBase* unique_data,
			                             GameTickT due_game_tick);

			///
			/// \brief Registers callback which will be called after the specified game ticks pass
			/// \param deferred Implements virtual function on_defer_time_elapsed
			/// \param elapse_game_tick Callback will be called in game ticks from now
			/// \return Index of registered callback, use this to remove the callback later
			DeferralEntry RegisterFromTick(const data::IDeferred& deferred, data::UniqueDataBase* unique_data,
			                               GameTickT elapse_game_tick);

			///
			/// \brief Removes registered callback at game_tick at index
			void RemoveDeferral(DeferralEntry entry);

			///
			/// \brief Removes registered callback and sets entry index to 0
			void RemoveDeferralEntry(DeferralEntry& entry);

		private:
			GameTickT lastGameTick_ = 0;
			WorldData& worldData_;

		} deferralTimer{*this};

		// ======================================================================

		///
		/// \brief Calls callbacks for tile updates
		class UpdateDispatcher
		{
			using CallbackT = const data::IUpdateListener*;
			using CollectionT = std::vector<std::pair<WorldPair, CallbackT>>;

			using ContainerKeyT = std::tuple<WorldCoord, WorldCoord>;
			using ContainerT = std::unordered_map<ContainerKeyT, CollectionT, core::hash<ContainerKeyT>>;

			ContainerT container_;

		public:
			/// Current world coord, Registered world coord 
			using ListenerEntry = std::pair<WorldPair, WorldPair>;

			explicit UpdateDispatcher(WorldData& world_data)
				: worldData_(world_data) {
			}

			///
			/// \brief Registers proto_listener callback when target coords is updated, providing current coords
			ListenerEntry Register(WorldCoord current_world_x, WorldCoord current_world_y,
								   WorldCoord target_world_x, WorldCoord target_world_y,
								   const data::IUpdateListener& proto_listener);

			///
			/// \brief Registers proto_listener callback when target coords is updated, providing current coords
			ListenerEntry Register(const WorldPair& current_coords, const WorldPair& target_coords,
								   const data::IUpdateListener& proto_listener);

			///
			/// \brief Unregisters entry
			/// \return true if succeeded, false if failed
			bool Unregister(const ListenerEntry& entry);

			void Dispatch(WorldCoord world_x, WorldCoord world_y, data::UpdateType type);
			void Dispatch(const WorldPair& world_pair, data::UpdateType type);

		private:
			WorldData& worldData_;

		} updateDispatcher{*this};
	};
}

#endif //JACTORIO_INCLUDE_GAME_WORLD_WORLD_DATA_H
