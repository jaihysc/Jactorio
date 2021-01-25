// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "game/world/chunk.h"

#include "jactorioTests.h"

namespace jactorio::game
{
    TEST(Chunk, LogicCopy) {
        Chunk chunk_a(0, 0);
        chunk_a.GetLogicGroup(LogicGroup::conveyor)
            .push_back(&chunk_a.GetCTile(3, 4).GetLayer(TileLayer::base));

        auto chunk_b = chunk_a;
        EXPECT_EQ(chunk_b.GetLogicGroup(LogicGroup::conveyor)[0],
                  &chunk_b.GetCTile(3, 4).GetLayer(TileLayer::base));
    }

    TEST(Chunk, LogicMove) {
        Chunk chunk_a(0, 0);
        chunk_a.GetLogicGroup(LogicGroup::inserter)
            .push_back(&chunk_a.GetCTile(4, 3).GetLayer(TileLayer::resource));

        auto chunk_b = std::move(chunk_a);
        EXPECT_EQ(chunk_b.GetLogicGroup(LogicGroup::inserter)[0],
                  &chunk_b.GetCTile(4, 3).GetLayer(TileLayer::resource));
    }

    TEST(Chunk, GetCTile) {
        // GetCTile means Get Chunk Tile
        // GetTile already taken by WorldData

        Chunk chunk(4, 4);
        EXPECT_EQ(&chunk.Tiles()[23 * 32 + 12], &chunk.GetCTile({12, 23}));
    }

    TEST(Chunk, GetObjectLayer) {
        Chunk chunk_a{0, 0};

        // Should return the layer specified by the index of the enum objectLayer
        EXPECT_EQ(&chunk_a.GetOverlay(OverlayLayer::cursor), &chunk_a.overlays[0]);
    }

    TEST(Chunk, GetLogicGroup) {
        Chunk chunk{0, 0};

        // Should return the layer specified by the index of the enum objectLayer
        EXPECT_EQ(&chunk.GetLogicGroup(LogicGroup::conveyor), &chunk.logicGroups[0]);
    }

    TEST(Chunk, SerializeLogicGroups) {
        Chunk chunk{0, 0};

        chunk.GetLogicGroup(LogicGroup::inserter).push_back(&chunk.GetCTile(4, 10).GetLayer(TileLayer::entity));

        auto result        = TestSerializeDeserialize(chunk);
        auto& result_logic = result.GetLogicGroup(LogicGroup::inserter);

        ASSERT_EQ(result_logic.size(), 1);
        EXPECT_EQ(result_logic[0], &result.GetCTile(4, 10).GetLayer(TileLayer::entity));
    }
} // namespace jactorio::game
