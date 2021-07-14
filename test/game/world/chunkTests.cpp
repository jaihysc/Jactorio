// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "game/world/chunk.h"

#include "jactorioTests.h"

namespace jactorio::game
{
    TEST(Chunk, WorldCoordToChunkTileCoord) {
        EXPECT_EQ(Chunk::WorldCToChunkTileC(987654), 6);

        EXPECT_EQ(Chunk::WorldCToChunkTileC(-32), 0);
        EXPECT_EQ(Chunk::WorldCToChunkTileC(-1), 31);
        EXPECT_EQ(Chunk::WorldCToChunkTileC(-2), 30);

        EXPECT_EQ(Chunk::WorldCToChunkTileC(-33), 31);
    }

    TEST(Chunk, GetCTile) {
        // GetCTile means Get Chunk Tile
        // GetTile already taken by World

        Chunk chunk({4, 4});
        EXPECT_EQ(&chunk.Tiles(TileLayer::entity)[23 * 32 + 12], &chunk.GetCTile({12, 23}, TileLayer::entity));
    }

    // TEST(Chunk, GetOverlayLayer) {
    //     Chunk chunk_a{{0, 0}};
    //
    //     // Should return the layer specified by the index of the enum objectLayer
    //     EXPECT_EQ(&chunk_a.GetOverlay(OverlayLayer::debug), &chunk_a.overlays[0]);
    // }
} // namespace jactorio::game
