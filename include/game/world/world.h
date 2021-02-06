// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_WORLD_WORLD_H
#define JACTORIO_INCLUDE_GAME_WORLD_WORLD_H
#pragma once

#include <set>
#include <unordered_map>
#include <utility>

#include "jactorio.h"

#include "core/data_type.h"
#include "game/world/chunk.h"
#include "game/world/update_dispatcher.h"

namespace jactorio::proto
{
    enum class UpdateType;

    class IDeferred;
    class IUpdateListener;
} // namespace jactorio::proto

namespace jactorio::game
{
    ///
    /// Stores all data for a world
    class World
    {
        using LogicChunkContainerT       = std::vector<Chunk*>;
        using SerialLogicChunkContainerT = std::vector<ChunkCoord>;

    public:
        static constexpr auto kChunkWidth = Chunk::kChunkWidth;

        static ChunkCoordAxis WorldCToChunkC(WorldCoordAxis coord);
        static ChunkCoord WorldCToChunkC(const WorldCoord& coord);
        ///
        /// Chunk coord -> World coord at first tile of chunk
        static WorldCoordAxis ChunkCToWorldC(ChunkCoordAxis chunk_coord);
        static WorldCoord ChunkCToWorldC(const ChunkCoord& chunk_coord);

        static OverlayOffsetAxis WorldCToOverlayC(WorldCoordAxis coord);


        // World access

        World()  = default;
        ~World() = default;

        World(const World& other);
        World(World&& other) noexcept = default;

        World& operator=(World other) {
            swap(*this, other);
            return *this;
        }

        friend void swap(World& lhs, World& rhs) noexcept {
            using std::swap;
            swap(lhs.updateDispatcher, rhs.updateDispatcher);
            swap(lhs.worldChunks_, rhs.worldChunks_);
            swap(lhs.logicChunks_, rhs.logicChunks_);
            swap(lhs.worldGenSeed_, rhs.worldGenSeed_);
            swap(lhs.worldGenChunks_, rhs.worldGenChunks_);
        }


        ///
        /// \param args Additional arguments to be provided alongside chunk_x chunk_y to Chunk constructor
        /// \return Added chunk
        template <typename... TChunkArgs>
        Chunk& EmplaceChunk(ChunkCoordAxis chunk_x, ChunkCoordAxis chunk_y, TChunkArgs... args) {
            const auto& [it, success] = worldChunks_.emplace(std::piecewise_construct,
                                                             std::make_tuple(chunk_x, chunk_y),
                                                             std::make_tuple(chunk_x, chunk_y, args...));
            assert(success); // Attempted to insert at already existent location

            return it->second;
        }

        template <typename... TChunkArgs>
        Chunk& EmplaceChunk(const ChunkCoord& chunk_coord, TChunkArgs... args) {
            return EmplaceChunk(chunk_coord.x, chunk_coord.y, std::forward<TChunkArgs>()...);
        }

        ///
        /// Attempts to delete chunk at chunk_x, chunk_y
        void DeleteChunk(ChunkCoordAxis chunk_x, ChunkCoordAxis chunk_y);

        ///
        /// Clears chunk data, logic chunks and chunks awaiting generation
        /// \remark Ensure PrepareWorldClear was called prior to this
        void Clear();


        ///
        /// Retrieves a chunk in game world using chunk coordinates
        /// \return nullptr if no chunk exists
        J_NODISCARD Chunk* GetChunkC(ChunkCoordAxis chunk_x, ChunkCoordAxis chunk_y);

        ///
        /// Retrieves a chunk in game world using chunk coordinates
        /// \return nullptr if no chunk exists
        J_NODISCARD const Chunk* GetChunkC(ChunkCoordAxis chunk_x, ChunkCoordAxis chunk_y) const;


        ///
        /// Retrieves a chunk in game world using chunk coordinates
        /// \return nullptr if no chunk exists
        J_NODISCARD Chunk* GetChunkC(const ChunkCoord& chunk_pair);

        ///
        /// Retrieves a chunk in game world using chunk coordinates
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
        J_NODISCARD Chunk* GetChunkW(const WorldCoord& coord);

        ///
        /// Gets the chunk at the specified world coordinate
        /// \return nullptr if no chunk exists
        J_NODISCARD const Chunk* GetChunkW(const WorldCoord& coord) const;

        // ======================================================================

        ///
        /// Gets the tile at the specified world coordinate
        /// \return nullptr if no tile exists
        J_NODISCARD ChunkTile* GetTile(WorldCoordAxis world_x, WorldCoordAxis world_y);

        ///
        /// Gets the tile at the specified world coordinate
        /// \return nullptr if no tile exists
        J_NODISCARD const ChunkTile* GetTile(WorldCoordAxis world_x, WorldCoordAxis world_y) const;


        ///
        /// Gets the tile at the specified world coordinate
        /// \return nullptr if no tile exists
        J_NODISCARD ChunkTile* GetTile(const WorldCoord& coord);

        ///
        /// Gets the tile at the specified world coordinate
        /// \return nullptr if no tile exists
        J_NODISCARD const ChunkTile* GetTile(const WorldCoord& coord) const;

        // ======================================================================

        ///
        /// Gets top left tile for provided layer if is multi tile, otherwise itself if not a multi tile
        J_NODISCARD ChunkTile* GetTileTopLeft(const WorldCoord& coord, TileLayer layer);

        ///
        /// Gets top left tile for provided layer if is multi tile, otherwise itself if not a multi tile
        J_NODISCARD const ChunkTile* GetTileTopLeft(const WorldCoord& coord, TileLayer layer) const;

        ///
        /// Gets top left tile if is multi tile, otherwise itself if not a multi tile
        J_NODISCARD ChunkTile* GetTileTopLeft(WorldCoord coord, const ChunkTileLayer& chunk_tile_layer);

        ///
        /// Gets top left tile if is multi tile, otherwise itself if not a multi tile
        J_NODISCARD const ChunkTile* GetTileTopLeft(const WorldCoord& coord,
                                                    const ChunkTileLayer& chunk_tile_layer) const;


        ///
        /// Gets top left ChunkTileLayer at ChunkLayer is is multi tile, otherwise itself
        J_NODISCARD ChunkTileLayer* GetLayerTopLeft(const WorldCoord& coord, const TileLayer& tile_layer) noexcept;

        ///
        /// Gets top left ChunkTileLayer at ChunkLayer is is multi tile, otherwise itself
        J_NODISCARD const ChunkTileLayer* GetLayerTopLeft(const WorldCoord& coord,
                                                          const TileLayer& tile_layer) const noexcept;

        // ==============================================================
        // Logic chunk

        ///
        /// Adds a layer at coordinates to be considered for logic updates
        void LogicRegister(LogicGroup group, const WorldCoord& coord, TileLayer layer);

        ///
        /// Removes a layer at coordinates to be considered for logic updates
        /// w/ custom comparison func to remove
        void LogicRemove(LogicGroup group, const WorldCoord& coord, const std::function<bool(ChunkTileLayer*)>& pred);

        ///
        /// Removes a layer at coordinates to be considered for logic updates
        void LogicRemove(LogicGroup group, const WorldCoord& coord, TileLayer layer);


        ///
        /// Adds a chunk to be considered for logic updates, if the logic chunk already exists at Chunk*,
        /// a reference to the existing one will be returned
        /// \param chunk The chunk this logic chunk is associated with
        void LogicAddChunk(Chunk& chunk);

        ///
        /// Returns all the chunks which require logic updates
        J_NODISCARD LogicChunkContainerT& LogicGetChunks();
        J_NODISCARD const LogicChunkContainerT& LogicGetChunks() const;

        // ======================================================================
        // World generation

        void SetWorldGeneratorSeed(const int seed) {
            worldGenSeed_ = seed;
        }
        J_NODISCARD int GetWorldGeneratorSeed() const {
            return worldGenSeed_;
        }


        ///
        /// Queues a chunk to be generated at specified position
        /// \remark To be called from render thread only
        void QueueChunkGeneration(ChunkCoordAxis chunk_x, ChunkCoordAxis chunk_y) const;

        ///
        /// Takes first in from chunk generation queue and generates chunk
        /// Call once per logic loop tick to generate one chunk only, this keeps performance constant
        /// when generating large amounts of chunks
        void GenChunk(const data::PrototypeManager& proto, uint8_t amount = 1);


        // ======================================================================

        ///
        /// Forwards args to updateDispatcher.Dispatch itself being world data
        template <typename... TArgs>
        auto UpdateDispatch(const WorldCoord& coord, TArgs&&... args) {
            updateDispatcher.Dispatch(*this, coord, std::forward<TArgs>(args)...);
        }

        ///
        /// Forwards args to updateDispatcher.Dispatch itself being world data
        template <typename... TArgs>
        auto UpdateDispatch(TArgs&&... args) {
            updateDispatcher.Dispatch(*this, std::forward<TArgs>(args)...);
        }


        ///
        /// To be used after deserializing
        /// Steps through all chunks:
        /// Dispatches OnDeserialize(),
        /// Sets the top left tile for all multi tile tiles as its pointer cannot be serialized
        void DeserializePostProcess();


        CEREAL_LOAD(archive) {
            SerialLogicChunkContainerT logic_chunks;
            archive(updateDispatcher, worldChunks_, worldGenSeed_, logic_chunks);

            logicChunks_.clear();

            FromSerializeLogicChunkContainer(logic_chunks);
        }

        CEREAL_SAVE(archive) {
            auto logic_chunks = ToSerializeLogicChunkContainer();
            archive(updateDispatcher, worldChunks_, worldGenSeed_, logic_chunks);
        }


        UpdateDispatcher updateDispatcher;

    private:
        using ChunkKey    = std::tuple<ChunkCoordAxis, ChunkCoordAxis>;
        using ChunkHasher = core::hash<ChunkKey>;

        /// Chunks increment heading right and down
        std::unordered_map<ChunkKey, Chunk, ChunkHasher> worldChunks_;
        LogicChunkContainerT logicChunks_;


        int worldGenSeed_ = 1001;
        /// Stores whether or not a chunk is being generated, this gets cleared once all world generation is done
        mutable std::set<ChunkKey> worldGenChunks_;

        J_NODISCARD SerialLogicChunkContainerT ToSerializeLogicChunkContainer() const;
        void FromSerializeLogicChunkContainer(const SerialLogicChunkContainerT& serial_logic);
    };
} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_WORLD_WORLD_H
