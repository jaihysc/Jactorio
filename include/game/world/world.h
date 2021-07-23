// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_WORLD_WORLD_H
#define JACTORIO_INCLUDE_GAME_WORLD_WORLD_H
#pragma once

#include <set>
#include <unordered_map>
#include <utility>

#include "jactorio.h"

#include "core/data_type.h"
#include "core/dvector.h"
#include "game/world/chunk.h"
#include "game/world/logic_group.h"
#include "game/world/update_dispatcher.h"

namespace jactorio::proto
{
    enum class UpdateType;

    class IDeferred;
    class IUpdateListener;
    class Entity;
} // namespace jactorio::proto

namespace jactorio::game
{
    /// Represents entity registered for logic updates
    struct LogicObject
    {
        data::SerialProtoPtr<const proto::FrameworkBase> prototype  = nullptr;
        data::SerialUniqueDataPtr<proto::UniqueDataBase> uniqueData = nullptr;
        WorldCoord coord;

        CEREAL_SERIALIZE(archive) {
            archive(prototype, uniqueData, coord);
        }

        friend bool operator==(const LogicObject& lhs, const LogicObject& rhs) {
            return std::tie(lhs.prototype, lhs.uniqueData, lhs.coord) ==
                std::tie(rhs.prototype, rhs.uniqueData, rhs.coord);
        }

        friend bool operator!=(const LogicObject& lhs, const LogicObject& rhs) {
            return !(lhs == rhs);
        }
    };

    /// Stores all data for a world
    class World
    {
        using LogicListT                 = std::vector<LogicObject>;
        using SerialLogicChunkContainerT = std::vector<ChunkCoord>;

        /// Format: | 0 1 2 | 0 1 2 |
        ///         <1 tile >
        /// 0 1 2 are the different layers, layer 0 first
        /// Kept contiguous for rendering cache locality
        using TexCoordIdArrayT = std::array<SpriteTexCoordIndexT, Chunk::kChunkArea * kTileLayerCount>;

    public:
        static ChunkCoordAxis WorldCToChunkC(WorldCoordAxis coord);
        static ChunkCoord WorldCToChunkC(const WorldCoord& coord);
        /// Chunk coord -> World coord at first tile of chunk
        static WorldCoordAxis ChunkCToWorldC(ChunkCoordAxis chunk_coord);
        static WorldCoord ChunkCToWorldC(const ChunkCoord& chunk_coord);

        static OverlayOffsetAxis WorldCToOverlayC(WorldCoordAxis coord);
        static Position2<OverlayOffsetAxis> WorldCToOverlayC(const WorldCoord& coord);


        // World access

        /// Creates chunk
        /// \param args Additional arguments to be provided alongside chunk_x chunk_y to Chunk constructor
        /// \return Added chunk
        template <typename... TChunkArgs>
        Chunk& EmplaceChunk(const ChunkCoord& c_coord, TChunkArgs... args) {
            const auto& [it, success] = worldChunks_.emplace(
                std::piecewise_construct, std::make_tuple(c_coord.x, c_coord.y), std::make_tuple(c_coord, args...));
            assert(success); // Attempted to insert at already existent location

            /// Ensures there is an element available for provided coordinate axis
            auto fill_to_axis = [](auto& dvector, const ChunkCoordAxis coord_axis) {
                if (coord_axis < 0) {
                    dvector.reserve(-coord_axis * 2); // Avoid unnecessary resize if short on capacity
                    while (dvector.size_front() < -coord_axis) {
                        dvector.emplace_front();
                    }
                }
                else {
                    dvector.reserve((coord_axis + 1) * 2); // index 0 counts as element
                    while (dvector.size_back() < coord_axis + 1) {
                        dvector.emplace_back();
                    }
                }
            };

            fill_to_axis(chunkTexCoordIds_, c_coord.y);
            fill_to_axis(chunkTexCoordIds_[c_coord.y], c_coord.x);

            return it->second;
        }

        /// Attempts to delete chunk at chunk_x, chunk_y
        void DeleteChunk(const ChunkCoord& c_coord);

        /// Clears chunk data, logic chunks and chunks awaiting generation
        /// \remark Ensure PrepareWorldClear was called prior to this
        void Clear();


        /// Retrieves a chunk in game world using chunk coordinates
        /// \return nullptr if no chunk exists
        J_NODISCARD Chunk* GetChunkC(const ChunkCoord& c_coord);

        /// Retrieves a chunk in game world using chunk coordinates
        /// \return nullptr if no chunk exists
        J_NODISCARD const Chunk* GetChunkC(const ChunkCoord& c_coord) const;


        /// Gets the chunk at the specified world coordinate
        /// \return nullptr if no chunk exists
        J_NODISCARD Chunk* GetChunkW(const WorldCoord& coord);

        /// Gets the chunk at the specified world coordinate
        /// \return nullptr if no chunk exists
        J_NODISCARD const Chunk* GetChunkW(const WorldCoord& coord) const;


        // Get Tile

        /// Gets the tile at the specified world coordinate
        /// \return nullptr if no tile exists
        J_NODISCARD ChunkTile* GetTile(const WorldCoord& coord, TileLayer tlayer);

        /// Gets the tile at the specified world coordinate
        /// \return nullptr if no tile exists
        J_NODISCARD const ChunkTile* GetTile(const WorldCoord& coord, TileLayer tlayer) const;


        // Rendering methods


        /// \return First: Pointer to contiguous tex coord ids, starting at first tile of requested chunk,
        /// increments per tile right then down, then next chunk first tile
        /// \return Second: Number of chunks that can be read from the pointer
        J_NODISCARD std::pair<SpriteTexCoordIndexT*, int> GetChunkTexCoordIds(const ChunkCoord& c_coord) noexcept;

        /// \return First: Pointer to contiguous tex coord ids, starting at first tile of requested chunk,
        /// increments per tile right then down, then next chunk first tile
        /// \return Second: Number of chunks that can be read from the pointer
        J_NODISCARD std::pair<const SpriteTexCoordIndexT*, int> GetChunkTexCoordIds(
            const ChunkCoord& c_coord) const noexcept;


        /// \return tex coord id at given coord and layer
        J_NODISCARD SpriteTexCoordIndexT GetTexCoordId(const WorldCoord& coord, TileLayer layer) const noexcept;

        /// Sets tex coord id for given world coord at layer
        void SetTexCoordId(const WorldCoord& coord, TileLayer layer, SpriteTexCoordIndexT id) noexcept;

        /// Sets tex coord id for given chunk coord, chunk tile coord at layer
        void SetTexCoordId(const ChunkCoord& c_coord,
                           const ChunkTileCoord& ct_coord,
                           TileLayer layer,
                           SpriteTexCoordIndexT id) noexcept;

        /// Enables animation for multi-tile(if exists) at tile layer at coord
        /// - Uses animation offset from TileRenderer
        /// - All animations are enabled by default
        void EnableAnimation(const WorldCoord& coord, TileLayer tlayer) noexcept;
        /// Enables animation for multi-tile(if exists) at tile layer at coord
        /// - All animations are enabled by default
        void EnableAnimation(WorldCoord coord, TileLayer tlayer, SpriteTexCoordIndexT animation_offset) noexcept;

        /// Disables animation for multi-tile(if exists) at tile layer at coord
        /// - Uses animation offset from TileRenderer
        void DisableAnimation(const WorldCoord& coord, TileLayer tlayer) noexcept;
        /// Disables animation for multi-tile(if exists) at tile layer at coord
        void DisableAnimation(WorldCoord coord, TileLayer tlayer, SpriteTexCoordIndexT animation_offset) noexcept;


        // ======================================================================
        // Placement

        /// Determines if entity of given dimensions can be placed at specified coord
        /// \return true if a entity with the specified dimensions can be placed at coord
        J_NODISCARD bool PlaceLocationValid(const WorldCoord& coord, Dimension dimensions) const;

        /// Places entity at coord
        /// \return true if placed successfully
        bool Place(const WorldCoord& coord, Orientation orien, const proto::Entity& entity);

        /// Removes entity at coord
        /// \return true if removed successfully
        bool Remove(const WorldCoord& coord, Orientation orien);


        // ==============================================================
        // Logic updates

        /// Adds a tile layer at tlayer at coord to group to be considered for logic updates
        void LogicRegister(LogicGroup group, const WorldCoord& coord, TileLayer tlayer);

        /// Removes a group at tlayer at coord from being considered for logic updates
        void LogicRemove(LogicGroup group, const WorldCoord& coord, TileLayer tlayer);

        J_NODISCARD LogicListT& LogicGet(LogicGroup group);
        J_NODISCARD const LogicListT& LogicGet(LogicGroup group) const;

        // ======================================================================
        // World generation

        void SetWorldGeneratorSeed(const int seed) {
            worldGenSeed_ = seed;
        }
        J_NODISCARD int GetWorldGeneratorSeed() const {
            return worldGenSeed_;
        }


        /// Queues a chunk to be generated at specified position
        /// \remark To be called from render thread only
        void QueueChunkGeneration(const ChunkCoord& c_coord) const;

        /// Takes first in from chunk generation queue and generates chunk
        /// Call once per logic loop tick to generate one chunk only, this keeps performance constant
        /// when generating large amounts of chunks
        void GenChunk(const data::PrototypeManager& proto, uint8_t amount = 1);


        // ======================================================================

        /// Forwards args to updateDispatcher.Dispatch itself being world data
        template <typename... TArgs>
        auto UpdateDispatch(const WorldCoord& coord, TArgs&&... args) {
            updateDispatcher.Dispatch(*this, coord, std::forward<TArgs>(args)...);
        }

        /// Forwards args to updateDispatcher.Dispatch itself being world data
        template <typename... TArgs>
        auto UpdateDispatch(TArgs&&... args) {
            updateDispatcher.Dispatch(*this, std::forward<TArgs>(args)...);
        }


        /// To be used after deserializing
        /// Steps through all chunks:
        /// Dispatches OnDeserialize(),
        /// Sets the top left tile for all multi tile tiles as its pointer cannot be serialized
        void DeserializePostProcess();


        CEREAL_SERIALIZE(archive) {
            // NOTE: Unique data is only available after deserializing worldChunks_
            archive(updateDispatcher, chunkTexCoordIds_, worldChunks_, logicLists_, worldGenSeed_);
        }

        UpdateDispatcher updateDispatcher;

    private:
        using ChunkKey    = std::tuple<ChunkCoordAxis, ChunkCoordAxis>;
        using ChunkHasher = hash<ChunkKey>;

        DVector<DVector<TexCoordIdArrayT>> chunkTexCoordIds_;

        /// Chunks increment heading right and down
        std::unordered_map<ChunkKey, Chunk, ChunkHasher> worldChunks_;
        std::array<LogicListT, kLogicGroupCount> logicLists_;


        int worldGenSeed_ = 1001;
        /// Stores whether or not a chunk is being generated, this gets cleared once all world generation is done
        mutable std::set<ChunkKey> worldGenChunks_;
    };
} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_WORLD_WORLD_H
