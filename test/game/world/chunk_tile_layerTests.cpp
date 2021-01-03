// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "game/world/chunk_tile_layer.h"

#include "data/cereal/register_type.h" // Just has to be included somewhere once
#include "jactorioTests.h"

namespace jactorio::game
{
    class ChunkTileLayerTest : public testing::Test
    {
    protected:
        proto::ContainerEntity proto_;

        ///
        /// Calls TestSetupMultiTileProp with proto_
        /// ctl: Set prototype with provided orientation and multi tile properties
        void SetupMultiTileProp(ChunkTileLayer& ctl, const Orientation orientation, const MultiTileData& mt_data) {
            TestSetupMultiTileProp(ctl, orientation, mt_data, proto_);
        }
    };

    TEST_F(ChunkTileLayerTest, Copy) {
        ChunkTileLayer top_left;
        SetupMultiTileProp(top_left, Orientation::up, {5, 2});
        top_left.MakeUniqueData<proto::ContainerEntityData>(32);

        ChunkTileLayer ctl;
        ctl.SetupMultiTile(4, top_left);
        SetupMultiTileProp(ctl, Orientation::up, {5, 2});
        ctl.SetOrientation(Orientation::right);

        // Top left
        {
            const ChunkTileLayer copy{top_left};
            EXPECT_EQ(copy.GetMultiTileIndex(), 0);

            EXPECT_EQ(copy.GetMultiTileData().height, 2);
            EXPECT_EQ(copy.GetMultiTileData(), top_left.GetMultiTileData());

            EXPECT_EQ(copy.GetUniqueData<proto::ContainerEntityData>()->inventory.size(), 32);

            EXPECT_EQ(copy.GetOrientation(), Orientation::right);
        }
        // Non top left
        {
            const ChunkTileLayer copy = ctl;

            EXPECT_EQ(copy.GetMultiTileIndex(), 4);

            EXPECT_EQ(copy.GetMultiTileData().span, 5);
            EXPECT_EQ(copy.GetMultiTileData(), top_left.GetMultiTileData());

            EXPECT_EQ(copy.GetTopLeftLayer(), nullptr); // top left layer not copied
        }
    }

    TEST_F(ChunkTileLayerTest, Move) {
        ChunkTileLayer top_left;
        SetupMultiTileProp(top_left, Orientation::up, {5, 2});
        top_left.MakeUniqueData<proto::ContainerEntityData>(32);

        ChunkTileLayer ctl;
        ctl.SetupMultiTile(4, top_left);
        SetupMultiTileProp(ctl, Orientation::up, {5, 2});
        ctl.SetOrientation(Orientation::right);

        // Top left
        {
            const ChunkTileLayer move_to = std::move(top_left);

            EXPECT_EQ(top_left.GetUniqueData<proto::ContainerEntityData>(), nullptr); // Gave ownership
            EXPECT_NE(move_to.GetUniqueData<proto::ContainerEntityData>(), nullptr);  // Took ownership

            EXPECT_EQ(move_to.GetOrientation(), Orientation::right);
        }
        // Non top left
        {
            const ChunkTileLayer move_to = std::move(ctl);

            EXPECT_EQ(move_to.GetTopLeftLayer(), &top_left);

            EXPECT_EQ(move_to.GetOrientation(), Orientation::right);
        }
    }

    TEST_F(ChunkTileLayerTest, SetOrientationIsTopLeft) {
        ChunkTileLayer top_left;

        top_left.SetOrientation(Orientation::right);
        EXPECT_EQ(top_left.GetOrientation(), Orientation::right);
    }

    TEST_F(ChunkTileLayerTest, SetOrientationNonTopLeft) {
        ChunkTileLayer top_left;
        SetupMultiTileProp(top_left, Orientation::up, {2, 1});

        ChunkTileLayer ctl;
        ctl.SetupMultiTile(1, top_left);

        ctl.SetOrientation(Orientation::right);

        EXPECT_EQ(top_left.GetOrientation(), Orientation::right);
        EXPECT_EQ(ctl.GetOrientation(), Orientation::right);
    }

    TEST_F(ChunkTileLayerTest, GetSetPrototype) {
        ChunkTileLayer top_left;

        ChunkTileLayer ctl;
        ctl.SetupMultiTile(1, top_left);
        SetupMultiTileProp(ctl, Orientation::right, {2, 3});

        EXPECT_EQ(ctl.GetPrototype(), &proto_);
        EXPECT_EQ(top_left.GetOrientation(), Orientation::right);
    }

    TEST_F(ChunkTileLayerTest, GetSetNullPrototype) {
        ChunkTileLayer ctl;

        ctl.SetPrototype(Orientation::right, &proto_);
        ctl.SetPrototype(nullptr);

        EXPECT_EQ(ctl.GetPrototype(), nullptr);
        EXPECT_EQ(ctl.GetOrientation(), Orientation::right);
    }

    TEST_F(ChunkTileLayerTest, GetUniqueData) {
        ChunkTileLayer top_left;
        SetupMultiTileProp(top_left, Orientation::up, {2, 3});
        top_left.MakeUniqueData<proto::ContainerEntityData>(10);

        ChunkTileLayer ctl;
        ctl.SetupMultiTile(3, top_left);
        SetupMultiTileProp(ctl, Orientation::up, {2, 3});


        EXPECT_EQ(top_left.GetUniqueData(), ctl.GetUniqueData());
        EXPECT_EQ(top_left.GetUniqueData(), top_left.GetUniqueDataLocal());
    }


    TEST_F(ChunkTileLayerTest, IsTopLeft) {
        {
            const ChunkTileLayer ctl;
            EXPECT_TRUE(ctl.IsTopLeft());
        }
        {
            ChunkTileLayer ctl;
            EXPECT_TRUE(ctl.IsTopLeft());
        }
        {
            ChunkTileLayer ctl;
            SetupMultiTileProp(ctl, Orientation::up, {1, 2});
            EXPECT_TRUE(ctl.IsTopLeft());
        }
        {
            ChunkTileLayer top_left;
            ChunkTileLayer ctl;
            ctl.SetupMultiTile(4, top_left);
            EXPECT_FALSE(ctl.IsTopLeft());
        }
    }

    TEST_F(ChunkTileLayerTest, IsMultiTile) {
        {
            const ChunkTileLayer ctl;
            EXPECT_FALSE(ctl.IsMultiTile()); // No prototype data
        }
        {
            ChunkTileLayer ctl;
            SetupMultiTileProp(ctl, Orientation::up, {1, 1});
            EXPECT_FALSE(ctl.IsMultiTile()); // multiTileIndex is 0, multiTileData is 1, 1
        }
        {
            ChunkTileLayer ctl;
            SetupMultiTileProp(ctl, Orientation::up, {1, 2});
            EXPECT_TRUE(ctl.IsMultiTile());
        }
        {
            ChunkTileLayer top_left;
            ChunkTileLayer ctl;
            ctl.SetupMultiTile(4, top_left);
            SetupMultiTileProp(ctl, Orientation::up, {1, 2});
            EXPECT_TRUE(ctl.IsMultiTile());
        }
    }

    TEST_F(ChunkTileLayerTest, IsMultiTileTopLeft) {
        {
            ChunkTileLayer ctl;
            EXPECT_FALSE(ctl.IsMultiTileTopLeft());
        }

        {
            ChunkTileLayer ctl;
            SetupMultiTileProp(ctl, Orientation::up, {1, 2});
            EXPECT_TRUE(ctl.IsMultiTileTopLeft());
        }
        {
            ChunkTileLayer top_left;
            ChunkTileLayer ctl;
            ctl.SetupMultiTile(4, top_left);
            EXPECT_FALSE(ctl.IsMultiTileTopLeft());
        }
    }

    TEST_F(ChunkTileLayerTest, IsNonTopLeftMultiTile) {
        {
            ChunkTileLayer ctl;
            EXPECT_FALSE(ctl.IsNonTopLeftMultiTile());
        }

        {
            ChunkTileLayer ctl;
            SetupMultiTileProp(ctl, Orientation::up, {1, 2});
            EXPECT_FALSE(ctl.IsNonTopLeftMultiTile());
        }
        {
            ChunkTileLayer top_left;
            ChunkTileLayer ctl;
            ctl.SetupMultiTile(4, top_left);
            SetupMultiTileProp(ctl, Orientation::up, {1, 2});
            EXPECT_TRUE(ctl.IsNonTopLeftMultiTile());
        }
    }

    TEST_F(ChunkTileLayerTest, OverrideMultiTileData) {
        // Both should return the same multi tile data
        ChunkTileLayer ctl;
        SetupMultiTileProp(ctl, Orientation::up, {12, 32});
        SetupMultiTileProp(ctl, Orientation::up, {3, 30});

        const auto& data = ctl.GetMultiTileData();

        EXPECT_EQ(data.span, 3);
        EXPECT_EQ(data.height, 30);
    }

    TEST_F(ChunkTileLayerTest, GetMultiTileData) {
        ChunkTileLayer first;
        SetupMultiTileProp(first, Orientation::up, {12, 32});

        ASSERT_TRUE(first.HasMultiTileData());
        const auto& data_1 = first.GetMultiTileData();

        EXPECT_EQ(data_1.span, proto_.GetWidth());
        EXPECT_EQ(data_1.height, proto_.GetHeight());


        ChunkTileLayer second;
        second.SetupMultiTile(1, first);
        ASSERT_FALSE(second.HasMultiTileData());
    }


    TEST_F(ChunkTileLayerTest, SetupMultiTile) {
        ChunkTileLayer top_left;
        ChunkTileLayer ctl;

        ctl.SetupMultiTile(1, top_left);
        EXPECT_EQ(ctl.GetMultiTileIndex(), 1);
        EXPECT_EQ(ctl.GetTopLeftLayer(), &top_left);
    }

    TEST_F(ChunkTileLayerTest, AdjustToTopleft) {
        ChunkTileLayer top_left;
        ChunkTileLayer ctl;
        ctl.SetupMultiTile(5, top_left);
        SetupMultiTileProp(ctl, Orientation::up, {3, 2});

        int x = 0;
        int y = 0;
        ctl.AdjustToTopLeft(x, y);

        WorldCoord coord;
        ctl.AdjustToTopLeft(coord);

        EXPECT_EQ(x, -2);
        EXPECT_EQ(y, -1);

        EXPECT_EQ(coord, WorldCoord(-2, -1));
    }

    TEST_F(ChunkTileLayerTest, AdjustToTopleftNonMultiTile) {
        const ChunkTileLayer ctl;

        int x = 0;
        int y = 0;
        ctl.AdjustToTopLeft(x, y);

        EXPECT_EQ(x, 0);
        EXPECT_EQ(y, 0);
    }

    TEST_F(ChunkTileLayerTest, GetOffsetX) {
        ChunkTileLayer top_left;
        ChunkTileLayer ctl;
        ctl.SetupMultiTile(19, top_left);
        SetupMultiTileProp(ctl, Orientation::up, {10, 2});

        EXPECT_EQ(ctl.GetOffsetX(), 9);
    }

    TEST_F(ChunkTileLayerTest, GetOffsetY) {
        ChunkTileLayer top_left;
        ChunkTileLayer ctl;
        ctl.SetupMultiTile(20, top_left);
        SetupMultiTileProp(ctl, Orientation::up, {5, 2});

        EXPECT_EQ(ctl.GetOffsetY(), 4);
    }

    TEST_F(ChunkTileLayerTest, Serialize) {
        data::PrototypeManager proto_manager;
        data::UniqueDataManager unique_manager;

        data::active_prototype_manager   = &proto_manager;
        data::active_unique_data_manager = &unique_manager;

        auto& proto = proto_manager.AddProto<proto::ContainerEntity>();

        ChunkTileLayer top_left;
        TestSetupMultiTileProp(top_left, Orientation::up, {2, 2}, proto);
        top_left.MakeUniqueData<proto::ContainerEntityData>(10);

        ChunkTileLayer bot_right;
        bot_right.SetupMultiTile(3, top_left);
        TestSetupMultiTileProp(bot_right, Orientation::up, {2, 2}, proto);
        bot_right.SetOrientation(Orientation::right);

        proto_manager.GenerateRelocationTable();

        // ======================================================================
        auto result_tl = TestSerializeDeserialize(top_left);
        auto result_br = TestSerializeDeserialize(bot_right);

        EXPECT_EQ(result_tl.GetPrototype(), &proto);
        ASSERT_NE(result_tl.GetUniqueData(), nullptr);
        EXPECT_EQ(result_tl.GetUniqueData()->internalId, 1);

        EXPECT_EQ(result_tl.GetMultiTileData().span, 2);
        EXPECT_EQ(result_tl.GetMultiTileData().height, 2);

        EXPECT_EQ(result_tl.GetOrientation(), Orientation::right);

        EXPECT_EQ(result_br.GetPrototype(), &proto);
        EXPECT_EQ(result_br.GetTopLeftLayer(), nullptr);

        EXPECT_EQ(result_br.GetMultiTileData().span, 2);
        EXPECT_EQ(result_br.GetMultiTileData().height, 2);


        EXPECT_EQ(unique_manager.GetDebugInfo().dataEntries.size(), 1);
    }
} // namespace jactorio::game
