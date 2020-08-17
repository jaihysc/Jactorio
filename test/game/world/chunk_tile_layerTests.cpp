// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "game/world/chunk_tile_layer.h"

#include "jactorioTests.h"
#include "data/prototype/container_entity.h"

namespace jactorio::game
{
	class ChunkTileLayerTest : public testing::Test
	{
	protected:
		data::ContainerEntity proto_;

		void SetupMultiTileProp(ChunkTileLayer& ctl, const MultiTileData& mt_data) {
			TestSetupMultiTileProp(ctl, mt_data, proto_);
		}
	};

	TEST_F(ChunkTileLayerTest, CopyNonMultiTile) {
		ChunkTileLayer ctl{};

		// Copy construct
		{
			const ChunkTileLayer ctl_copy{ctl};
			EXPECT_EQ(ctl_copy.multiTileIndex, 0);
		}
		// Copy assignment
		{
			ctl.multiTileIndex            = 4;
			const ChunkTileLayer ctl_copy = ctl;
			EXPECT_EQ(ctl_copy.multiTileIndex, 4);
		}
	}

	TEST_F(ChunkTileLayerTest, CopyMultiTile) {
		ChunkTileLayer ctl{};
		SetupMultiTileProp(ctl, {1, 2});
		ctl.multiTileIndex = 3;

		// Copy construct
		{
			const ChunkTileLayer ctl_copy{ctl};
			EXPECT_EQ(ctl_copy.multiTileIndex, 3);

			EXPECT_EQ(ctl.GetMultiTileData().span, 1);
			EXPECT_EQ(ctl_copy.GetMultiTileData(), ctl.GetMultiTileData());
		}
		// Copy assignment
		{
			ctl.multiTileIndex = 6;

			const ChunkTileLayer ctl_copy = ctl;
			EXPECT_EQ(ctl_copy.multiTileIndex, 6);

			EXPECT_EQ(ctl.GetMultiTileData().height, 2);
			EXPECT_EQ(ctl_copy.GetMultiTileData(), ctl.GetMultiTileData());
		}
	}

	TEST_F(ChunkTileLayerTest, IsMultiTile) {
		{
			const ChunkTileLayer ctl{};
			EXPECT_FALSE(ctl.IsMultiTile());  // No prototype data
		}
		{
			ChunkTileLayer ctl{};
			SetupMultiTileProp(ctl, {1, 1});
			ctl.multiTileIndex = 0;
			EXPECT_FALSE(ctl.IsMultiTile());  // multiTileIndex is 0, multiTileData is 1, 1
		}
		{
			ChunkTileLayer ctl{};
			SetupMultiTileProp(ctl, {1, 2});
			ctl.multiTileIndex = 0;
			EXPECT_TRUE(ctl.IsMultiTile());
		}
		{
			ChunkTileLayer ctl{};
			SetupMultiTileProp(ctl, {1, 2});
			ctl.multiTileIndex = 4;
			EXPECT_TRUE(ctl.IsMultiTile());
		}
	}

	TEST_F(ChunkTileLayerTest, IsMultiTileTopLeft) {
		{
			ChunkTileLayer ctl{};
			ctl.multiTileIndex = 0;
			EXPECT_FALSE(ctl.IsMultiTileTopLeft());
		}

		{
			ChunkTileLayer ctl{};
			ctl.multiTileIndex = 0;
			SetupMultiTileProp(ctl, {1, 2});
			EXPECT_TRUE(ctl.IsMultiTileTopLeft());
		}
		{
			ChunkTileLayer ctl{};
			ctl.multiTileIndex = 4;
			EXPECT_FALSE(ctl.IsMultiTileTopLeft());
		}
	}

	TEST_F(ChunkTileLayerTest, OverrideMultiTileData) {
		// Both should return the same multi tile data
		ChunkTileLayer ctl{};
		SetupMultiTileProp(ctl, {12, 32});
		SetupMultiTileProp(ctl, {3, 30});

		const auto& data = ctl.GetMultiTileData();

		EXPECT_EQ(data.span, 3);
		EXPECT_EQ(data.height, 30);
	}

	TEST_F(ChunkTileLayerTest, GetMultiTileData) {
		ChunkTileLayer first{};
		SetupMultiTileProp(first, {12, 32});

		ASSERT_TRUE(first.HasMultiTileData());
		const auto& data_1 = first.GetMultiTileData();

		EXPECT_EQ(data_1.span, proto_.tileWidth);
		EXPECT_EQ(data_1.height, proto_.tileHeight);


		ChunkTileLayer second{};
		second.multiTileIndex = 1;
		ASSERT_FALSE(second.HasMultiTileData());
	}

	TEST_F(ChunkTileLayerTest, AdjustToTopleft) {
		ChunkTileLayer ctl{};
		SetupMultiTileProp(ctl, {3, 2});
		ctl.multiTileIndex = 5;

		int x = 0;
		int y = 0;
		ctl.AdjustToTopLeft(x, y);

		EXPECT_EQ(x, -2);
		EXPECT_EQ(y, -1);
	}

	TEST_F(ChunkTileLayerTest, AdjustToTopleftNonMultiTile) {
		const ChunkTileLayer ctl{};

		int x = 0;
		int y = 0;
		ctl.AdjustToTopLeft(x, y);

		EXPECT_EQ(x, 0);
		EXPECT_EQ(y, 0);
	}

	TEST_F(ChunkTileLayerTest, GetOffsetX) {
		ChunkTileLayer ctl{};
		SetupMultiTileProp(ctl, {10, 2});
		ctl.multiTileIndex = 19;

		EXPECT_EQ(ctl.GetOffsetX(), 9);
	}

	TEST_F(ChunkTileLayerTest, GetOffsetY) {
		ChunkTileLayer ctl{};
		SetupMultiTileProp(ctl, {5, 2});
		ctl.multiTileIndex = 20;

		EXPECT_EQ(ctl.GetOffsetY(), 4);
	}
}
