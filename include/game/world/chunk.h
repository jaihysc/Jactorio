// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_WORLD_CHUNK_H
#define JACTORIO_INCLUDE_GAME_WORLD_CHUNK_H
#pragma once

#include <array>
#include <limits>
#include <vector>

#include "jactorio.h"

#include "data/cereal/serialize.h"
#include "game/world/chunk_tile.h"
#include "game/world/overlay_element.h"
#include "game/world/tile_layer.h"

#include <cereal/types/array.hpp>
#include <cereal/types/vector.hpp>

namespace jactorio::game
{
    /// A chunk within the game
    ///
    /// Made up of tiles and objects:
    ///		tiles: Has 32 x 32, fixed grid location
    ///		overlays: Has no set amount, can exist anywhere on chunk
    class Chunk
    {
    public:
        using OverlayContainerT    = std::vector<OverlayElement>;
        using LogicGroupContainerT = std::vector<ChunkTile*>;

        static constexpr uint8_t kChunkWidth = 32;
        static constexpr uint16_t kChunkArea = static_cast<uint16_t>(kChunkWidth) * kChunkWidth;

    private:
        using TileArrayT    = std::array<ChunkTile, kChunkArea>;
        using OverlayArrayT = std::array<OverlayContainerT, kOverlayLayerCount>;

    public:
        /// \remark For cereal deserialization only
        Chunk() = default;
        /// Default initialization of chunk tiles
        explicit Chunk(const ChunkCoord& c_coord) : position_(c_coord) {}

        J_NODISCARD static ChunkTileCoordAxis WorldCToChunkTileC(WorldCoordAxis coord);
        J_NODISCARD static ChunkTileCoord WorldCToChunkTileC(const WorldCoord& coord);


        J_NODISCARD ChunkCoord GetPosition() const {
            return position_;
        }

        /// Tiles at provided tlayer
        J_NODISCARD TileArrayT& Tiles(TileLayer tlayer) noexcept;
        J_NODISCARD const TileArrayT& Tiles(TileLayer tlayer) const noexcept;

        /// Gets tile at x, y offset from top left of chunk
        J_NODISCARD ChunkTile& GetCTile(const ChunkTileCoord& coord, TileLayer tlayer) noexcept;
        J_NODISCARD const ChunkTile& GetCTile(const ChunkTileCoord& coord, TileLayer tlayer) const noexcept;

        // Overlays - Rendered without being fixed to a tile position

        OverlayContainerT& GetOverlay(OverlayLayer layer);
        J_NODISCARD const OverlayContainerT& GetOverlay(OverlayLayer layer) const;

        CEREAL_SERIALIZE(archive) {
            archive(position_, layers_);
        }

        OverlayArrayT overlays;

    private:
        ChunkCoord position_;
        std::array<TileArrayT, kTileLayerCount> layers_;
    };
} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_WORLD_CHUNK_H
