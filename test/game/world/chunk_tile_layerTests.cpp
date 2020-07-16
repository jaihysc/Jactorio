// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "game/world/chunk_tile_layer.h"

namespace jactorio::game
{
	TEST(ChunkTileLayer, CopyNonMultiTile) {
		// Since the top left tile copy is unknown, it is set as nullptr

		const ChunkTileLayer ctl{};

		// Copy construct
		{
			const ChunkTileLayer ctl_copy{ctl};
			EXPECT_FALSE(ctl_copy.IsMultiTile());
		}
		// Copy assignment
		{
			const ChunkTileLayer ctl_copy = ctl;
			EXPECT_FALSE(ctl_copy.IsMultiTile());
		}
	}

	TEST(ChunkTileLayer, CopyMultiTile) {
		// Since the top left tile copy is unknown, it is set as nullptr
		ChunkTileLayer parent{};
		parent.multiTileIndex = 0;
		parent.InitMultiTileProp(1, 2);

		ChunkTileLayer ctl{};
		ctl.multiTileIndex = 3;
		ctl.SetMultiTileParent(&parent);

		// Copy construct
		{
			ChunkTileLayer ctl_copy{ctl};
			EXPECT_NE(ctl_copy.multiTileIndex, 0);

			ctl_copy.multiTileIndex = 0;  // Set index to 0 so .is_multi_tile() does not assert false
			EXPECT_FALSE(ctl_copy.IsMultiTile());  // Not considered multi tile since it does not point to parent
		}
		// Copy assignment
		{
			ChunkTileLayer ctl_copy = ctl;
			EXPECT_NE(ctl_copy.multiTileIndex, 0);

			ctl_copy.multiTileIndex = 0;  // Set index to 0 so .is_multi_tile() does not assert false
			EXPECT_FALSE(ctl_copy.IsMultiTile());  // Not considered multi tile since it does not point to parent
		}
	}

	TEST(ChunkTileLayer, CopyMultiTileTopLeft) {
		ChunkTileLayer ctl{};
		ctl.multiTileIndex = 0;
		ctl.InitMultiTileProp(1, 2);

		// Copy construct
		{
			const ChunkTileLayer ctl_copy{ctl};
			EXPECT_NE(&ctl_copy.GetMultiTileData(), &ctl.GetMultiTileData());
		}
		// Copy assignment
		{
			const ChunkTileLayer ctl_copy = ctl;
			EXPECT_NE(&ctl_copy.GetMultiTileData(), &ctl.GetMultiTileData());
		}
	}

	TEST(ChunkTileLayer, IsMultiTile) {
		{
			ChunkTileLayer ctl{};
			ctl.multiTileIndex = 0;
			EXPECT_FALSE(ctl.IsMultiTile());
		}

		{
			ChunkTileLayer ctl{};
			ctl.multiTileIndex = 0;
			ctl.InitMultiTileProp(1, 2);
			EXPECT_TRUE(ctl.IsMultiTile());
		}
		{
			ChunkTileLayer ctl{};
			ctl.multiTileIndex = 4;
			ctl.SetMultiTileParent(&ctl);  // Some random address
			EXPECT_TRUE(ctl.IsMultiTile());
		}
	}

	TEST(ChunkTileLayer, IsMultiTileTopLeft) {
		{
			ChunkTileLayer ctl{};
			ctl.multiTileIndex = 0;
			EXPECT_FALSE(ctl.IsMultiTileTopLeft());
		}

		{
			ChunkTileLayer ctl{};
			ctl.multiTileIndex = 0;
			ctl.InitMultiTileProp(1, 2);
			EXPECT_TRUE(ctl.IsMultiTileTopLeft());
		}
		{
			ChunkTileLayer ctl{};
			ctl.multiTileIndex = 4;
			ctl.SetMultiTileParent(&ctl);  // Some random address
			EXPECT_FALSE(ctl.IsMultiTileTopLeft());
		}
	}


	TEST(ChunkTileLayer, GetMultiTileTopLeft) {
		ChunkTileLayer first{};
		EXPECT_EQ(&first.GetMultiTileTopLeft(), &first);  // Returns self if not multi tile

		first.InitMultiTileProp(12, 32);

		ChunkTileLayer second{};
		second.multiTileIndex = 1;
		second.SetMultiTileParent(&first);


		EXPECT_EQ(&first.GetMultiTileTopLeft(), &first);
		EXPECT_EQ(&second.GetMultiTileTopLeft(), &first);
	}

	/*
	TEST(ChunkTileLayer, GetMultiTileTopLeftNonMultitile) {
		jactorio::game::ChunkTileLayer first{};

		EXPECT_EQ(first.GetMultiTileTopLeft(), &first);
	}
	*/

	TEST(ChunkTileLayer, SetMultiTileParent) {
		ChunkTileLayer first{};
		ChunkTileLayer second{};
		second.multiTileIndex = 1;
		second.SetMultiTileParent(&first);

		EXPECT_EQ(second.GetMultiTileParent(), &first);
	}

	TEST(ChunkTileLayer, OverrideMultiTileData) {
		// Both should return the same multi tile data
		ChunkTileLayer first{};
		first.InitMultiTileProp(12, 32);
		first.InitMultiTileProp(3, 30);

		MultiTileData& data = first.GetMultiTileData();

		EXPECT_EQ(data.multiTileSpan, 3);
		EXPECT_EQ(data.multiTileHeight, 30);
	}

	TEST(ChunkTileLayer, GetMultiTileData) {
		// Both should return the same multi tile data
		ChunkTileLayer first{};
		first.InitMultiTileProp(12, 32);

		ChunkTileLayer second{};
		second.multiTileIndex = 1;
		second.SetMultiTileParent(&first);


		MultiTileData& data_1 = first.GetMultiTileData();
		MultiTileData& data_2 = second.GetMultiTileData();

		EXPECT_EQ(&data_1, &data_2);
		EXPECT_NE(&data_1, nullptr);
	}

	TEST(ChunkTileLayer, AdjustToTopleft) {
		ChunkTileLayer parent{};
		parent.InitMultiTileProp(3, 2);

		ChunkTileLayer ctl{};
		ctl.multiTileIndex = 5;
		ctl.SetMultiTileParent(&parent);

		int x = 0;
		int y = 0;
		ctl.AdjustToTopLeft(x, y);

		EXPECT_EQ(x, -2);
		EXPECT_EQ(y, -1);
	}

	TEST(ChunkTileLayer, AdjustToTopleftNonMultiTile) {
		ChunkTileLayer ctl{};

		int x = 0;
		int y = 0;
		ctl.AdjustToTopLeft(x, y);

		EXPECT_EQ(x, 0);
		EXPECT_EQ(y, 0);
	}

	TEST(ChunkTileLayer, GetOffsetX) {
		ChunkTileLayer parent{};
		parent.InitMultiTileProp(10, 2);

		ChunkTileLayer ctl{};
		ctl.multiTileIndex = 19;
		ctl.SetMultiTileParent(&parent);

		EXPECT_EQ(ctl.GetOffsetX(), 9);
	}

	TEST(ChunkTileLayer, GetOffsetY) {
		ChunkTileLayer parent{};
		parent.InitMultiTileProp(5, 2);

		ChunkTileLayer ctl{};
		ctl.multiTileIndex = 20;
		ctl.SetMultiTileParent(&parent);

		EXPECT_EQ(ctl.GetOffsetY(), 4);
	}
}
