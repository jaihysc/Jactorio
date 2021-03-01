// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "game/world/chunk_tile.h"

#include "data/cereal/register_type.h" // Just has to be included somewhere once
#include "jactorioTests.h"

namespace jactorio::game
{
    class ChunkTileTest : public testing::Test
    {
    protected:
        proto::ContainerEntity proto_;
    };

    TEST_F(ChunkTileTest, CopyTopLeft) {
        // Since orientation is right, width and height are swapped
        proto_.SetDimension({2, 5});

        ChunkTile top_left;
        top_left.SetPrototype(Orientation::right, proto_);
        top_left.MakeUniqueData<proto::ContainerEntityData>(32);

        const ChunkTile copy{top_left};
        EXPECT_EQ(copy.GetMultiTileIndex(), 0);

        EXPECT_EQ(copy.GetDimension().span, 5);
        EXPECT_EQ(copy.GetDimension().height, 2);
        EXPECT_EQ(copy.GetDimension(), top_left.GetDimension());

        EXPECT_EQ(copy.GetUniqueData<proto::ContainerEntityData>()->inventory.Size(), 32);

        EXPECT_EQ(copy.GetOrientation(), Orientation::right);
    }

    TEST_F(ChunkTileTest, CopyNonTopLeft) {
        // Since orientation is right, width and height are swapped
        proto_.SetDimension({2, 5});

        ChunkTile top_left;
        ChunkTile tile;
        tile.SetupMultiTile(4, top_left);
        tile.SetPrototype(Orientation::right, proto_);

        const ChunkTile copy = tile;

        EXPECT_EQ(copy.GetMultiTileIndex(), 4);

        EXPECT_EQ(copy.GetDimension().span, 5);
        EXPECT_EQ(copy.GetDimension().height, 2);

        EXPECT_EQ(copy.GetTopLeft(), nullptr); // top left tile not copied
    }

    TEST_F(ChunkTileTest, MoveTopLeft) {
        ChunkTile top_left;
        top_left.MakeUniqueData<proto::ContainerEntityData>(32);

        const ChunkTile move_to = std::move(top_left);

        EXPECT_EQ(top_left.GetUniqueData<proto::ContainerEntityData>(), nullptr); // Gave ownership
        EXPECT_NE(move_to.GetUniqueData<proto::ContainerEntityData>(), nullptr);  // Took ownership

        EXPECT_EQ(move_to.GetOrientation(), Orientation::up);
    }

    TEST_F(ChunkTileTest, MoveNonTopLeft) {
        ChunkTile top_left;
        ChunkTile tile;
        tile.SetupMultiTile(4, top_left);
        tile.SetPrototype(Orientation::right, proto_);

        const ChunkTile move_to = std::move(tile);

        EXPECT_EQ(move_to.GetTopLeft(), &top_left);
        EXPECT_EQ(move_to.GetOrientation(), Orientation::right);
    }

    TEST_F(ChunkTileTest, ClearTopLeft) {
        ChunkTile tile;
        tile.SetPrototype(Orientation::up, proto_);
        tile.MakeUniqueData<proto::ContainerEntityData>();

        tile.Clear();

        EXPECT_EQ(tile.GetPrototype(), nullptr);
        EXPECT_EQ(tile.GetUniqueData(), nullptr);
        EXPECT_EQ(tile.GetMultiTileIndex(), 0);
    }

    TEST_F(ChunkTileTest, ClearNonTopLeft) {
        ChunkTile top_left;
        ChunkTile tile;
        tile.SetupMultiTile(2, top_left);
        tile.SetPrototype(Orientation::up, proto_);

        tile.Clear();

        EXPECT_EQ(tile.GetPrototype(), nullptr);
        EXPECT_EQ(tile.GetUniqueData(), nullptr);
        EXPECT_EQ(tile.GetMultiTileIndex(), 0);
    }

    TEST_F(ChunkTileTest, GetSetOrientation) {
        ChunkTile top_left;
        top_left.SetPrototype(Orientation::right, proto_);

        EXPECT_EQ(top_left.GetOrientation(), Orientation::right);
    }

    TEST_F(ChunkTileTest, GetSetPrototype) {
        ChunkTile tile;

        tile.SetPrototype(Orientation::right, proto_);

        EXPECT_EQ(tile.GetPrototype(), &proto_);
        EXPECT_EQ(tile.GetOrientation(), Orientation::right);


        tile.SetPrototype(Orientation::up, nullptr);

        EXPECT_EQ(tile.GetPrototype(), nullptr);
        EXPECT_EQ(tile.GetOrientation(), Orientation::up);


        tile.SetPrototype(Orientation::right, proto_);
        tile.SetPrototype(nullptr);

        EXPECT_EQ(tile.GetPrototype(), nullptr);
    }

    TEST_F(ChunkTileTest, GetUniqueData) {
        ChunkTile top_left;
        top_left.MakeUniqueData<proto::ContainerEntityData>(10);

        ChunkTile tile;
        tile.SetupMultiTile(3, top_left);


        EXPECT_EQ(top_left.GetUniqueData(), tile.GetUniqueData());
    }


    TEST_F(ChunkTileTest, IsTopLeft) {
        {
            const ChunkTile tile;
            EXPECT_TRUE(tile.IsTopLeft());
        }
        {
            ChunkTile top_left;
            ChunkTile tile;
            tile.SetupMultiTile(4, top_left);
            EXPECT_FALSE(tile.IsTopLeft());
        }
    }

    TEST_F(ChunkTileTest, IsMultiTile) {
        {
            const ChunkTile tile;
            EXPECT_FALSE(tile.IsMultiTile());
        }
        {
            // Top left has to look at prototype to determine if it is multi tile
            proto_.SetWidth(3);

            ChunkTile tile;
            tile.SetPrototype(Orientation::up, proto_);
            EXPECT_TRUE(tile.IsMultiTile());
        }
        {
            ChunkTile top_left;
            ChunkTile tile;
            tile.SetupMultiTile(1, top_left);
            EXPECT_TRUE(tile.IsMultiTile());
        }
    }

    TEST_F(ChunkTileTest, IsMultiTileTopLeft) {
        {
            ChunkTile tile;
            EXPECT_FALSE(tile.IsMultiTileTopLeft());
        }

        {
            proto_.SetWidth(3);

            ChunkTile tile;
            tile.SetPrototype(Orientation::up, proto_);
            EXPECT_TRUE(tile.IsMultiTileTopLeft());
        }
        {
            ChunkTile top_left;
            ChunkTile tile;
            tile.SetupMultiTile(4, top_left);
            EXPECT_FALSE(tile.IsMultiTileTopLeft());
        }
    }

    TEST_F(ChunkTileTest, IsNonTopLeft) {
        {
            ChunkTile tile;
            EXPECT_FALSE(tile.IsNonTopLeft());
        }

        {
            ChunkTile top_left;
            ChunkTile tile;
            tile.SetupMultiTile(1, top_left);
            EXPECT_TRUE(tile.IsNonTopLeft());
        }
    }

    TEST_F(ChunkTileTest, GetDimensions) {
        const ChunkTile first;
        const auto dimens = first.GetDimension();

        EXPECT_EQ(dimens.span, 1);
        EXPECT_EQ(dimens.height, 1);
    }

    TEST_F(ChunkTileTest, GetDimensionsMultiTile) {
        proto::ContainerEntity container;
        container.SetDimension({2, 3});

        ChunkTile first;
        first.SetPrototype(Orientation::right, &container);

        const auto& dimens = first.GetDimension();

        EXPECT_EQ(dimens.span, container.GetWidth(Orientation::right));
        EXPECT_EQ(dimens.height, container.GetHeight(Orientation::right));
    }


    TEST_F(ChunkTileTest, SetupMultiTile) {
        ChunkTile top_left;
        ChunkTile tile;

        tile.SetupMultiTile(1, top_left);
        EXPECT_EQ(tile.GetMultiTileIndex(), 1);
        EXPECT_EQ(tile.GetTopLeft(), &top_left);
    }

    TEST_F(ChunkTileTest, GetTopLeftAlreadyTopleft) {
        ChunkTile tile;
        EXPECT_EQ(tile.GetTopLeft(), &tile);
    }

    TEST_F(ChunkTileTest, AdjustToTopleft) {
        proto_.SetWidth(3);

        ChunkTile top_left;
        ChunkTile tile;
        tile.SetPrototype(Orientation::up, proto_);
        tile.SetupMultiTile(5, top_left);

        Position2<int> p;
        Position2Increment(tile, p, -2);

        EXPECT_EQ(p.x, 4);
        EXPECT_EQ(p.y, 2);
    }

    TEST_F(ChunkTileTest, AdjustToTopleftNonMultiTile) {
        const ChunkTile tile;

        Position2<int> p;
        Position2Increment(tile, p, 100);

        EXPECT_EQ(p.x, 0);
        EXPECT_EQ(p.y, 0);
    }

    TEST_F(ChunkTileTest, GetOffsetX) {
        proto_.SetWidth(10);

        ChunkTile top_left;
        ChunkTile tile;
        tile.SetPrototype(Orientation::up, proto_);
        tile.SetupMultiTile(19, top_left);

        EXPECT_EQ(tile.GetOffsetX(), 9);
    }

    TEST_F(ChunkTileTest, GetOffsetY) {
        proto_.SetWidth(5);

        ChunkTile top_left;
        ChunkTile tile;
        tile.SetPrototype(Orientation::up, proto_);
        tile.SetupMultiTile(20, top_left);

        EXPECT_EQ(tile.GetOffsetY(), 4);
    }

    TEST_F(ChunkTileTest, Serialize) {
        data::PrototypeManager proto;
        data::UniqueDataManager unique;

        data::active_prototype_manager   = &proto;
        data::active_unique_data_manager = &unique;

        auto& container = proto.Make<proto::ContainerEntity>();
        container.SetDimension({2, 3}); // Width and height are flipped since orientation is right

        ChunkTile top_left;
        top_left.SetPrototype(Orientation::right, container);
        top_left.MakeUniqueData<proto::ContainerEntityData>(10);

        ChunkTile bot_right;
        bot_right.SetupMultiTile(3, top_left);
        bot_right.SetPrototype(Orientation::right, container);

        proto.GenerateRelocationTable();

        // ======================================================================
        auto result_tl = TestSerializeDeserialize(top_left);
        auto result_br = TestSerializeDeserialize(bot_right);

        EXPECT_EQ(result_tl.GetPrototype(), &container);
        ASSERT_NE(result_tl.GetUniqueData(), nullptr);
        EXPECT_EQ(result_tl.GetUniqueData()->internalId, 1);

        EXPECT_EQ(result_tl.GetDimension().span, 3);
        EXPECT_EQ(result_tl.GetDimension().height, 2);

        EXPECT_EQ(result_tl.GetOrientation(), Orientation::right);

        EXPECT_EQ(result_br.GetPrototype(), &container);
        EXPECT_EQ(result_br.GetTopLeft(), nullptr);

        EXPECT_EQ(result_br.GetDimension().span, 3);
        EXPECT_EQ(result_br.GetDimension().height, 2);


        EXPECT_EQ(unique.GetDebugInfo().dataEntries.size(), 1);
    }
} // namespace jactorio::game
