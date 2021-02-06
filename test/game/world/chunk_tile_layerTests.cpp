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
    };

    TEST_F(ChunkTileLayerTest, CopyTopLeft) {
        // Since orientation is right, width and height are swapped
        proto_.SetWidth(2);
        proto_.SetHeight(5);

        ChunkTileLayer top_left;
        top_left.SetPrototype(Orientation::right, proto_);
        top_left.MakeUniqueData<proto::ContainerEntityData>(32);

        const ChunkTileLayer copy{top_left};
        EXPECT_EQ(copy.GetMultiTileIndex(), 0);

        EXPECT_EQ(copy.GetDimensions().span, 5);
        EXPECT_EQ(copy.GetDimensions().height, 2);
        EXPECT_EQ(copy.GetDimensions(), top_left.GetDimensions());

        EXPECT_EQ(copy.GetUniqueData<proto::ContainerEntityData>()->inventory.size(), 32);

        EXPECT_EQ(copy.GetOrientation(), Orientation::right);
    }

    TEST_F(ChunkTileLayerTest, CopyNonTopLeft) {
        // Since orientation is right, width and height are swapped
        proto_.SetWidth(2);
        proto_.SetHeight(5);

        ChunkTileLayer top_left;
        ChunkTileLayer ctl;
        ctl.SetupMultiTile(4, top_left);
        ctl.SetPrototype(Orientation::right, proto_);

        const ChunkTileLayer copy = ctl;

        EXPECT_EQ(copy.GetMultiTileIndex(), 4);

        EXPECT_EQ(copy.GetDimensions().span, 5);
        EXPECT_EQ(copy.GetDimensions().height, 2);

        EXPECT_EQ(copy.GetTopLeftLayer(), nullptr); // top left layer not copied
    }

    TEST_F(ChunkTileLayerTest, CopyAssignTopLeft) {
        ChunkTileLayer top_left;
        top_left.SetPrototype(Orientation::up, proto_);
        top_left.MakeUniqueData<proto::ContainerEntityData>(20);

        ChunkTileLayer other;
        other = top_left;

        EXPECT_EQ(other.GetUniqueData<proto::ContainerEntityData>()->inventory.size(), 20);
    }

    TEST_F(ChunkTileLayerTest, MoveAssignNonTopLeft) {
        ChunkTileLayer top_left;
        ChunkTileLayer ctl;
        ctl.SetPrototype(Orientation::up, proto_);
        ctl.SetupMultiTile(3, top_left);

        ChunkTileLayer other;
        other = std::move(ctl);

        EXPECT_EQ(other.GetTopLeftLayer(), &top_left);
    }

    TEST_F(ChunkTileLayerTest, MoveTopLeft) {
        ChunkTileLayer top_left;
        top_left.MakeUniqueData<proto::ContainerEntityData>(32);

        const ChunkTileLayer move_to = std::move(top_left);

        EXPECT_EQ(top_left.GetUniqueData<proto::ContainerEntityData>(), nullptr); // Gave ownership
        EXPECT_NE(move_to.GetUniqueData<proto::ContainerEntityData>(), nullptr);  // Took ownership

        EXPECT_EQ(move_to.GetOrientation(), Orientation::up);
    }

    TEST_F(ChunkTileLayerTest, MoveNonTopLeft) {
        ChunkTileLayer top_left;
        ChunkTileLayer ctl;
        ctl.SetupMultiTile(4, top_left);
        ctl.SetPrototype(Orientation::right, proto_);

        const ChunkTileLayer move_to = std::move(ctl);

        EXPECT_EQ(move_to.GetTopLeftLayer(), &top_left);
        EXPECT_EQ(move_to.GetOrientation(), Orientation::right);
    }

    TEST_F(ChunkTileLayerTest, ClearTopLeft) {
        ChunkTileLayer ctl;
        ctl.SetPrototype(Orientation::up, proto_);
        ctl.MakeUniqueData<proto::ContainerEntityData>();

        ctl.Clear();

        EXPECT_EQ(ctl.GetPrototype(), nullptr);
        EXPECT_EQ(ctl.GetUniqueData(), nullptr);
        EXPECT_EQ(ctl.GetMultiTileIndex(), 0);
    }

    TEST_F(ChunkTileLayerTest, ClearNonTopLeft) {
        ChunkTileLayer top_left;
        ChunkTileLayer ctl;
        ctl.SetupMultiTile(2, top_left);
        ctl.SetPrototype(Orientation::up, proto_);

        ctl.Clear();

        EXPECT_EQ(ctl.GetPrototype(), nullptr);
        EXPECT_EQ(ctl.GetUniqueData(), nullptr);
        EXPECT_EQ(ctl.GetMultiTileIndex(), 0);
    }

    TEST_F(ChunkTileLayerTest, GetSetOrientation) {
        ChunkTileLayer top_left;
        top_left.SetPrototype(Orientation::right, proto_);

        EXPECT_EQ(top_left.GetOrientation(), Orientation::right);
    }

    TEST_F(ChunkTileLayerTest, GetSetPrototype) {
        ChunkTileLayer ctl;

        ctl.SetPrototype(Orientation::right, proto_);

        EXPECT_EQ(ctl.GetPrototype(), &proto_);
        EXPECT_EQ(ctl.GetOrientation(), Orientation::right);


        ctl.SetPrototype(Orientation::up, nullptr);

        EXPECT_EQ(ctl.GetPrototype(), nullptr);
        EXPECT_EQ(ctl.GetOrientation(), Orientation::up);


        ctl.SetPrototype(Orientation::right, proto_);
        ctl.SetPrototype(nullptr);

        EXPECT_EQ(ctl.GetPrototype(), nullptr);
    }

    TEST_F(ChunkTileLayerTest, GetUniqueData) {
        ChunkTileLayer top_left;
        top_left.MakeUniqueData<proto::ContainerEntityData>(10);

        ChunkTileLayer ctl;
        ctl.SetupMultiTile(3, top_left);


        EXPECT_EQ(top_left.GetUniqueData(), ctl.GetUniqueData());
    }


    TEST_F(ChunkTileLayerTest, IsTopLeft) {
        {
            const ChunkTileLayer ctl;
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
            EXPECT_FALSE(ctl.IsMultiTile());
        }
        {
            // Top left has to look at prototype to determine if it is multi tile
            proto_.SetWidth(3);

            ChunkTileLayer ctl;
            ctl.SetPrototype(Orientation::up, proto_);
            EXPECT_TRUE(ctl.IsMultiTile());
        }
        {
            ChunkTileLayer top_left;
            ChunkTileLayer ctl;
            ctl.SetupMultiTile(1, top_left);
            EXPECT_TRUE(ctl.IsMultiTile());
        }
    }

    TEST_F(ChunkTileLayerTest, IsMultiTileTopLeft) {
        {
            ChunkTileLayer ctl;
            EXPECT_FALSE(ctl.IsMultiTileTopLeft());
        }

        {
            proto_.SetWidth(3);

            ChunkTileLayer ctl;
            ctl.SetPrototype(Orientation::up, proto_);
            EXPECT_TRUE(ctl.IsMultiTileTopLeft());
        }
        {
            ChunkTileLayer top_left;
            ChunkTileLayer ctl;
            ctl.SetupMultiTile(4, top_left);
            EXPECT_FALSE(ctl.IsMultiTileTopLeft());
        }
    }

    TEST_F(ChunkTileLayerTest, IsNonTopLeft) {
        {
            ChunkTileLayer ctl;
            EXPECT_FALSE(ctl.IsNonTopLeft());
        }

        {
            ChunkTileLayer top_left;
            ChunkTileLayer ctl;
            ctl.SetupMultiTile(1, top_left);
            EXPECT_TRUE(ctl.IsNonTopLeft());
        }
    }

    TEST_F(ChunkTileLayerTest, GetDimensions) {
        const ChunkTileLayer first;
        const auto dimens = first.GetDimensions();

        EXPECT_EQ(dimens.span, 1);
        EXPECT_EQ(dimens.height, 1);
    }

    TEST_F(ChunkTileLayerTest, GetDimensionsMultiTile) {
        proto::ContainerEntity container;
        container.SetWidth(2);
        container.SetHeight(3);

        ChunkTileLayer first;
        first.SetPrototype(Orientation::right, &container);

        const auto& dimens = first.GetDimensions();

        EXPECT_EQ(dimens.span, container.GetWidth(Orientation::right));
        EXPECT_EQ(dimens.height, container.GetHeight(Orientation::right));
    }


    TEST_F(ChunkTileLayerTest, SetupMultiTile) {
        ChunkTileLayer top_left;
        ChunkTileLayer ctl;

        ctl.SetupMultiTile(1, top_left);
        EXPECT_EQ(ctl.GetMultiTileIndex(), 1);
        EXPECT_EQ(ctl.GetTopLeftLayer(), &top_left);
    }

    TEST_F(ChunkTileLayerTest, AdjustToTopleft) {
        proto_.SetWidth(3);

        ChunkTileLayer top_left;
        ChunkTileLayer ctl;
        ctl.SetPrototype(Orientation::up, proto_);
        ctl.SetupMultiTile(5, top_left);

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
        proto_.SetWidth(10);

        ChunkTileLayer top_left;
        ChunkTileLayer ctl;
        ctl.SetPrototype(Orientation::up, proto_);
        ctl.SetupMultiTile(19, top_left);

        EXPECT_EQ(ctl.GetOffsetX(), 9);
    }

    TEST_F(ChunkTileLayerTest, GetOffsetY) {
        proto_.SetWidth(5);

        ChunkTileLayer top_left;
        ChunkTileLayer ctl;
        ctl.SetPrototype(Orientation::up, proto_);
        ctl.SetupMultiTile(20, top_left);

        EXPECT_EQ(ctl.GetOffsetY(), 4);
    }

    TEST_F(ChunkTileLayerTest, Serialize) {
        data::PrototypeManager proto;
        data::UniqueDataManager unique_manager;

        data::active_prototype_manager   = &proto;
        data::active_unique_data_manager = &unique_manager;

        auto& container = proto.Make<proto::ContainerEntity>();
        container.SetWidth(2); // Width and height are flipped since orientation is right
        container.SetHeight(3);

        ChunkTileLayer top_left;
        top_left.SetPrototype(Orientation::right, container);
        top_left.MakeUniqueData<proto::ContainerEntityData>(10);

        ChunkTileLayer bot_right;
        bot_right.SetupMultiTile(3, top_left);
        bot_right.SetPrototype(Orientation::right, container);

        proto.GenerateRelocationTable();

        // ======================================================================
        auto result_tl = TestSerializeDeserialize(top_left);
        auto result_br = TestSerializeDeserialize(bot_right);

        EXPECT_EQ(result_tl.GetPrototype(), &container);
        ASSERT_NE(result_tl.GetUniqueData(), nullptr);
        EXPECT_EQ(result_tl.GetUniqueData()->internalId, 1);

        EXPECT_EQ(result_tl.GetDimensions().span, 3);
        EXPECT_EQ(result_tl.GetDimensions().height, 2);

        EXPECT_EQ(result_tl.GetOrientation(), Orientation::right);

        EXPECT_EQ(result_br.GetPrototype(), &container);
        EXPECT_EQ(result_br.GetTopLeftLayer(), nullptr);

        EXPECT_EQ(result_br.GetDimensions().span, 3);
        EXPECT_EQ(result_br.GetDimensions().height, 2);


        EXPECT_EQ(unique_manager.GetDebugInfo().dataEntries.size(), 1);
    }
} // namespace jactorio::game
