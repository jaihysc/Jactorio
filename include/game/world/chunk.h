// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_WORLD_CHUNK_H
#define JACTORIO_INCLUDE_GAME_WORLD_CHUNK_H
#pragma once

#include "jactorio.h"

#include <array>
#include <vector>

#include "data/cereal/serialize.h"
#include "game/world/chunk_tile.h"
#include "game/world/overlay_element.h"

#include <cereal/types/array.hpp>
#include <cereal/types/vector.hpp>

namespace jactorio::game
{
    ///
    /// A chunk within the game
    ///
    /// Made up of tiles and objects:
    ///		tiles: Has 32 x 32, fixed grid location
    ///		objects: Has no set amount, can exist anywhere on chunk
    class Chunk
    {
    public:
        using OverlayContainerT    = std::vector<OverlayElement>;
        using LogicGroupContainerT = std::vector<ChunkTileLayer*>;


        static constexpr uint8_t kChunkWidth = 32;
        static constexpr uint16_t kChunkArea = static_cast<uint16_t>(kChunkWidth) * kChunkWidth;

        enum class LogicGroup
        {
            transport_line = 0,
            inserter,
            count_
        };

        static constexpr auto kLogicGroupCount = static_cast<int>(LogicGroup::count_);

    private:
        using TileArrayT       = std::array<ChunkTile, kChunkArea>;
        using OverlayArrayT    = std::array<OverlayContainerT, kOverlayLayerCount>;
        using LogicGroupArrayT = std::array<LogicGroupContainerT, kLogicGroupCount>;


        struct TileLayerInfo
        {
            ChunkTileCoord coord;
            TileLayer tileLayer;

            CEREAL_SERIALIZE(archive) {
                archive(coord, tileLayer);
            }
        };

        /// LogicGroupArrayT converted to this to be serialized
        using SerialLogicGroupArrayT = std::array<std::vector<TileLayerInfo>, kLogicGroupCount>;

    public:
        ///
        /// \remark For cereal deserialization only
        Chunk() = default;

        ///
        /// Default initialization of chunk tiles
        Chunk(const ChunkCoordAxis chunk_x, const ChunkCoordAxis chunk_y) : position_({chunk_x, chunk_y}) {}

        ~Chunk() = default;

        Chunk(const Chunk& other);
        Chunk(Chunk&& other) noexcept;

        Chunk& operator=(Chunk other) {
            swap(*this, other);
            return *this;
        }

        friend void swap(Chunk& lhs, Chunk& rhs) noexcept {
            using std::swap;
            swap(lhs.overlays, rhs.overlays);
            swap(lhs.logicGroups, rhs.logicGroups);
            swap(lhs.position_, rhs.position_);
            swap(lhs.tiles_, rhs.tiles_);
        }


        // ======================================================================


        J_NODISCARD ChunkCoord GetPosition() const {
            return position_;
        }

        J_NODISCARD TileArrayT& Tiles();
        J_NODISCARD const TileArrayT& Tiles() const;

        ///
        /// Gets tile at x, y offset from top left of chunk
        J_NODISCARD ChunkTile& GetCTile(ChunkTileCoordAxis x, ChunkTileCoordAxis y);
        J_NODISCARD const ChunkTile& GetCTile(ChunkTileCoordAxis x, ChunkTileCoordAxis y) const;

        J_NODISCARD ChunkTile& GetCTile(const ChunkTileCoord& coord);
        J_NODISCARD const ChunkTile& GetCTile(const ChunkTileCoord& coord) const;


        // Overlays - Rendered without being fixed to a tile position

        OverlayContainerT& GetOverlay(OverlayLayer layer);
        J_NODISCARD const OverlayContainerT& GetOverlay(OverlayLayer layer) const;


        // Items requiring logic updates

        J_NODISCARD LogicGroupContainerT& GetLogicGroup(LogicGroup layer);
        J_NODISCARD const LogicGroupContainerT& GetLogicGroup(LogicGroup layer) const;


        CEREAL_LOAD(archive) {
            SerialLogicGroupArrayT serial_logic;
            archive(position_, tiles_, serial_logic);

            FromSerializeLogicGroupArray(serial_logic);
        }

        CEREAL_SAVE(archive) {
            auto serial_logic = ToSerializeLogicGroupArray();
            archive(position_, tiles_, serial_logic);
        }


        OverlayArrayT overlays;

        /// Holds pointer to UniqueData at tile requiring logic update
        LogicGroupArrayT logicGroups;

    private:
        ChunkCoord position_;
        TileArrayT tiles_;

        ///
        /// Other chunk has logic entries pointing to tiles within itself,
        /// this will recreate the entries, pointing to this chunk's tiles
        void ResolveLogicEntries(const Chunk& other) noexcept;

        J_NODISCARD TileLayerInfo GetLayerInfo(const ChunkTileLayer& ctl) const noexcept;

        J_NODISCARD SerialLogicGroupArrayT ToSerializeLogicGroupArray() const;
        void FromSerializeLogicGroupArray(const SerialLogicGroupArrayT& serial_logic);
    };
} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_WORLD_CHUNK_H
