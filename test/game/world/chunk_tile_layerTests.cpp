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
			EXPECT_EQ(ctl_copy.GetMultiTileIndex(), 0);
		}
		// Copy assignment
		{
			ctl.SetMultiTileIndex(4);
			const ChunkTileLayer ctl_copy = ctl;
			EXPECT_EQ(ctl_copy.GetMultiTileIndex(), 4);
		}
	}

	TEST_F(ChunkTileLayerTest, CopyMultiTile) {
		ChunkTileLayer ctl{};
		SetupMultiTileProp(ctl, {1, 2});
		ctl.MakeUniqueData<data::ContainerEntityData>(32);

		// Copy construct
		{
			const ChunkTileLayer ctl_copy{ctl};
			EXPECT_EQ(ctl_copy.GetMultiTileIndex(), 0);

			EXPECT_EQ(ctl_copy.GetMultiTileData().height, 2);
			EXPECT_EQ(ctl_copy.GetMultiTileData(), ctl.GetMultiTileData());
			
		}
		ctl.SetMultiTileIndex(3);
		{
			const ChunkTileLayer ctl_copy{ctl};
			EXPECT_EQ(ctl_copy.GetMultiTileIndex(), 3);

			EXPECT_EQ(ctl_copy.GetMultiTileData().span, 1);
			EXPECT_EQ(ctl_copy.GetMultiTileData(), ctl.GetMultiTileData());

			EXPECT_EQ(ctl_copy.GetTopLeftLayer(), nullptr);  // Unique data/top left layer not copied
		}
		// Copy assignment
		{
			ctl.SetMultiTileIndex(6);

			const ChunkTileLayer ctl_copy = ctl;
			EXPECT_EQ(ctl_copy.GetMultiTileIndex(), 6);

			EXPECT_EQ(ctl_copy.GetMultiTileData(), ctl.GetMultiTileData());
			EXPECT_EQ(ctl_copy.GetTopLeftLayer(), nullptr);  // Unique data/top left layer not copied
		}
	}


	TEST_F(ChunkTileLayerTest, GetUniqueData) {
		ChunkTileLayer top_left{};
		SetupMultiTileProp(top_left, {2, 3});
		top_left.MakeUniqueData<data::ContainerEntityData>(10);

		ChunkTileLayer ctl{};
		SetupMultiTileProp(ctl, {2, 3});
		ctl.SetMultiTileIndex(3);
		ctl.SetTopLeftLayer(top_left);
		

		EXPECT_EQ(top_left.GetUniqueData(), ctl.GetUniqueData());
		EXPECT_EQ(top_left.GetUniqueData(), top_left.GetUniqueDataLocal());
	}


	TEST_F(ChunkTileLayerTest, IsMultiTile) {
		{
			const ChunkTileLayer ctl{};
			EXPECT_FALSE(ctl.IsMultiTile());  // No prototype data
		}
		{
			ChunkTileLayer ctl{};
			SetupMultiTileProp(ctl, {1, 1});
			ctl.SetMultiTileIndex(0);
			EXPECT_FALSE(ctl.IsMultiTile());  // multiTileIndex is 0, multiTileData is 1, 1
		}
		{
			ChunkTileLayer ctl{};
			SetupMultiTileProp(ctl, {1, 2});
			ctl.SetMultiTileIndex(0);
			EXPECT_TRUE(ctl.IsMultiTile());
		}
		{
			ChunkTileLayer ctl{};
			SetupMultiTileProp(ctl, {1, 2});
			ctl.SetMultiTileIndex(4);
			EXPECT_TRUE(ctl.IsMultiTile());
		}
	}

	TEST_F(ChunkTileLayerTest, IsMultiTileTopLeft) {
		{
			ChunkTileLayer ctl{};
			ctl.SetMultiTileIndex(0);
			EXPECT_FALSE(ctl.IsMultiTileTopLeft());
		}

		{
			ChunkTileLayer ctl{};
			ctl.SetMultiTileIndex(0);
			SetupMultiTileProp(ctl, {1, 2});
			EXPECT_TRUE(ctl.IsMultiTileTopLeft());
		}
		{
			ChunkTileLayer ctl{};
			ctl.SetMultiTileIndex(4);
			EXPECT_FALSE(ctl.IsMultiTileTopLeft());
		}
	}

	TEST_F(ChunkTileLayerTest, IsNonTopLeftMultiTile) {
		{
			ChunkTileLayer ctl{};
			ctl.SetMultiTileIndex(0);
			EXPECT_FALSE(ctl.IsNonTopLeftMultiTile());
		}

		{
			ChunkTileLayer ctl{};
			SetupMultiTileProp(ctl, {1, 2});
			ctl.SetMultiTileIndex(0);
			EXPECT_FALSE(ctl.IsNonTopLeftMultiTile());
		}
		{
			ChunkTileLayer ctl{};
			SetupMultiTileProp(ctl, {1, 2});
			ctl.SetMultiTileIndex(4);
			EXPECT_TRUE(ctl.IsNonTopLeftMultiTile());
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
		second.SetMultiTileIndex(1);
		ASSERT_FALSE(second.HasMultiTileData());
	}


	TEST_F(ChunkTileLayerTest, SetMultiTileIndex) {
		ChunkTileLayer ctl;
		ctl.MakeUniqueData<data::ContainerEntityData>(3);

		ctl.SetMultiTileIndex(1);  // unique data should be deleted 

		// Valgrind emits error if unique data not deleted
	}

	TEST_F(ChunkTileLayerTest, GetTopleftLayer) {
		ChunkTileLayer top_left;

		ChunkTileLayer ctl;
		SetupMultiTileProp(ctl, {1, 2});
		ctl.SetMultiTileIndex(1); 

		EXPECT_EQ(ctl.GetTopLeftLayer(), nullptr);

		ctl.SetTopLeftLayer(top_left);
		EXPECT_EQ(ctl.GetTopLeftLayer(), &top_left);
	}

	TEST_F(ChunkTileLayerTest, AdjustToTopleft) {
		ChunkTileLayer ctl{};
		SetupMultiTileProp(ctl, {3, 2});
		ctl.SetMultiTileIndex(5);

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
		ctl.SetMultiTileIndex(19);

		EXPECT_EQ(ctl.GetOffsetX(), 9);
	}

	TEST_F(ChunkTileLayerTest, GetOffsetY) {
		ChunkTileLayer ctl{};
		SetupMultiTileProp(ctl, {5, 2});
		ctl.SetMultiTileIndex(20);

		EXPECT_EQ(ctl.GetOffsetY(), 4);
	}
}
