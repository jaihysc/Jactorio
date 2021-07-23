// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/world/chunk.h"

using namespace jactorio;

ChunkTileCoordAxis game::Chunk::WorldCToChunkTileC(const WorldCoordAxis coord) {
    if (coord < 0) {
        return (kChunkWidth - 1) - (-coord - 1) % kChunkWidth;
    }
    return coord % kChunkWidth;
}
ChunkTileCoord game::Chunk::WorldCToChunkTileC(const WorldCoord& coord) {
    return {WorldCToChunkTileC(coord.x), WorldCToChunkTileC(coord.y)};
}

game::Chunk::TileArrayT& game::Chunk::Tiles(TileLayer tlayer) noexcept {
    return layers_[static_cast<int>(tlayer)];
}

const game::Chunk::TileArrayT& game::Chunk::Tiles(TileLayer tlayer) const noexcept {
    return layers_[static_cast<int>(tlayer)];
}

game::ChunkTile& game::Chunk::GetCTile(const ChunkTileCoord& coord, const TileLayer tlayer) noexcept {
    return const_cast<ChunkTile&>(static_cast<const Chunk*>(this)->GetCTile(coord, tlayer));
}

const game::ChunkTile& game::Chunk::GetCTile(const ChunkTileCoord& coord, const TileLayer tlayer) const noexcept {
    assert(coord.x < kChunkWidth);
    assert(coord.y < kChunkWidth);

    using IndexT = uint16_t;

    static_assert(std::numeric_limits<IndexT>::max() > kChunkArea);
    return Tiles(tlayer)[SafeCast<IndexT>(coord.y) * kChunkWidth + coord.x];
}

game::Chunk::OverlayContainerT& game::Chunk::GetOverlay(const OverlayLayer layer) {
    return const_cast<OverlayContainerT&>(static_cast<const Chunk*>(this)->GetOverlay(layer));
}

const game::Chunk::OverlayContainerT& game::Chunk::GetOverlay(OverlayLayer layer) const {
    return overlays[static_cast<OverlayArrayT::size_type>(layer)];
}
