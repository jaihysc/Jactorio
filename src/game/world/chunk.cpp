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

game::Chunk::OverlayContainerT& game::Chunk::GetOverlay(const OverlayLayer layer) {
    return const_cast<OverlayContainerT&>(static_cast<const Chunk*>(this)->GetOverlay(layer));
}

const game::Chunk::OverlayContainerT& game::Chunk::GetOverlay(OverlayLayer layer) const {
    return overlays[static_cast<OverlayArrayT::size_type>(layer)];
}
