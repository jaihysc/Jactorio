// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 04/03/2020

#include <gtest/gtest.h>

#include "game/world/chunk_tile_layer.h"

namespace game
{
	TEST(ChunkTileLayer, CopyNonMultiTile) {
		// Since the top left tile copy is unknown, it is set as nullptr

		const jactorio::game::ChunkTileLayer ctl{};

		// Copy construct
		{
			const jactorio::game::ChunkTileLayer ctl_copy{ctl};
			EXPECT_FALSE(ctl_copy.IsMultiTile());
		}
		// Copy assignment
		{
			const jactorio::game::ChunkTileLayer ctl_copy = ctl;
			EXPECT_FALSE(ctl_copy.IsMultiTile());
		}
	}

	TEST(ChunkTileLayer, CopyMultiTile) {
		// Since the top left tile copy is unknown, it is set as nullptr
		jactorio::game::ChunkTileLayer parent{};
		parent.multiTileIndex = 0;
		parent.InitMultiTileProp(1, 2);

		jactorio::game::ChunkTileLayer ctl{};
		ctl.multiTileIndex = 3;
		ctl.SetMultiTileParent(&parent);

		// Copy construct
		{
			jactorio::game::ChunkTileLayer ctl_copy{ctl};
			EXPECT_NE(ctl_copy.multiTileIndex, 0);

			ctl_copy.multiTileIndex = 0;  // Set index to 0 so .is_multi_tile() does not assert false
			EXPECT_FALSE(ctl_copy.IsMultiTile());  // Not considered multi tile since it does not point to parent
		}
		// Copy assignment
		{
			jactorio::game::ChunkTileLayer ctl_copy = ctl;
			EXPECT_NE(ctl_copy.multiTileIndex, 0);

			ctl_copy.multiTileIndex = 0;  // Set index to 0 so .is_multi_tile() does not assert false
			EXPECT_FALSE(ctl_copy.IsMultiTile());  // Not considered multi tile since it does not point to parent
		}
	}

	TEST(ChunkTileLayer, CopyMultiTileTopLeft) {
		jactorio::game::ChunkTileLayer ctl{};
		ctl.multiTileIndex = 0;
		ctl.InitMultiTileProp(1, 2);

		// Copy construct
		{
			const jactorio::game::ChunkTileLayer ctl_copy{ctl};
			EXPECT_NE(&ctl_copy.GetMultiTileData(), &ctl.GetMultiTileData());
		}
		// Copy assignment
		{
			const jactorio::game::ChunkTileLayer ctl_copy = ctl;
			EXPECT_NE(&ctl_copy.GetMultiTileData(), &ctl.GetMultiTileData());
		}
	}

	TEST(ChunkTileLayer, IsMultiTile) {
		{
			jactorio::game::ChunkTileLayer ctl{};
			ctl.multiTileIndex = 0;
			EXPECT_FALSE(ctl.IsMultiTile());
		}

		{
			jactorio::game::ChunkTileLayer ctl{};
			ctl.multiTileIndex = 0;
			ctl.InitMultiTileProp(1, 2);
			EXPECT_TRUE(ctl.IsMultiTile());
		}
		{
			jactorio::game::ChunkTileLayer ctl{};
			ctl.multiTileIndex = 4;
			ctl.SetMultiTileParent(&ctl);  // Some random address
			EXPECT_TRUE(ctl.IsMultiTile());
		}
	}

	TEST(ChunkTileLayer, IsMultiTileTopLeft) {
		{
			jactorio::game::ChunkTileLayer ctl{};
			ctl.multiTileIndex = 0;
			EXPECT_FALSE(ctl.IsMultiTileTopLeft());
		}

		{
			jactorio::game::ChunkTileLayer ctl{};
			ctl.multiTileIndex = 0;
			ctl.InitMultiTileProp(1, 2);
			EXPECT_TRUE(ctl.IsMultiTileTopLeft());
		}
		{
			jactorio::game::ChunkTileLayer ctl{};
			ctl.multiTileIndex = 4;
			ctl.SetMultiTileParent(&ctl);  // Some random address
			EXPECT_FALSE(ctl.IsMultiTileTopLeft());
		}
	}


	TEST(ChunkTileLayer, GetMultiTileTopLeft) {
		jactorio::game::ChunkTileLayer first{};
		first.InitMultiTileProp(12, 32);

		jactorio::game::ChunkTileLayer second{};
		second.multiTileIndex = 1;
		second.SetMultiTileParent(&first);


		EXPECT_EQ(first.GetMultiTileTopLeft(), &first);
		EXPECT_EQ(second.GetMultiTileTopLeft(), &first);
	}

	TEST(ChunkTileLayer, SetMultiTileParent) {
		jactorio::game::ChunkTileLayer first{};
		jactorio::game::ChunkTileLayer second{};
		second.multiTileIndex = 1;
		second.SetMultiTileParent(&first);

		EXPECT_EQ(second.GetMultiTileParent(), &first);
	}

	TEST(ChunkTileLayer, OverrideMultiTileData) {
		// Both should return the same multi tile data
		jactorio::game::ChunkTileLayer first{};
		first.InitMultiTileProp(12, 32);
		first.InitMultiTileProp(3, 30);

		jactorio::game::MultiTileData& data = first.GetMultiTileData();

		EXPECT_EQ(data.multiTileSpan, 3);
		EXPECT_EQ(data.multiTileHeight, 30);
	}

	TEST(ChunkTileLayer, GetMultiTileData) {
		// Both should return the same multi tile data
		jactorio::game::ChunkTileLayer first{};
		first.InitMultiTileProp(12, 32);

		jactorio::game::ChunkTileLayer second{};
		second.multiTileIndex = 1;
		second.SetMultiTileParent(&first);


		jactorio::game::MultiTileData& data_1 = first.GetMultiTileData();
		jactorio::game::MultiTileData& data_2 = second.GetMultiTileData();

		EXPECT_EQ(&data_1, &data_2);
		EXPECT_NE(&data_1, nullptr);
	}


	TEST(ChunkTileLayer, GetOffsetX) {
		jactorio::game::ChunkTileLayer parent{};
		parent.InitMultiTileProp(10, 2);

		jactorio::game::ChunkTileLayer ctl{};
		ctl.multiTileIndex = 19;
		ctl.SetMultiTileParent(&parent);

		EXPECT_EQ(ctl.GetOffsetX(), 9);
	}

	TEST(ChunkTileLayer, GetOffsetY) {
		jactorio::game::ChunkTileLayer parent{};
		parent.InitMultiTileProp(5, 2);

		jactorio::game::ChunkTileLayer ctl{};
		ctl.multiTileIndex = 20;
		ctl.SetMultiTileParent(&parent);

		EXPECT_EQ(ctl.GetOffsetY(), 4);
	}
}
