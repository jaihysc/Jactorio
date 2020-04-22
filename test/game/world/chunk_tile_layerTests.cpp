// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 04/03/2020

#include <gtest/gtest.h>

#include "game/world/chunk_tile_layer.h"

namespace game
{
	TEST(ChunkTileLayer, CopyNonMultiTile) {
		// Since the top left tile copy is unknown, it is set as nullptr

		const jactorio::game::Chunk_tile_layer ctl{};

		// Copy construct
		{
			const jactorio::game::Chunk_tile_layer ctl_copy{ctl};
			EXPECT_FALSE(ctl_copy.is_multi_tile());
		}
		// Copy assignment
		{
			const jactorio::game::Chunk_tile_layer ctl_copy = ctl;
			EXPECT_FALSE(ctl_copy.is_multi_tile());
		}
	}

	TEST(ChunkTileLayer, CopyMultiTile) {
		// Since the top left tile copy is unknown, it is set as nullptr
		jactorio::game::Chunk_tile_layer parent{};
		parent.multi_tile_index = 0;
		parent.init_multi_tile_prop(1, 2);

		jactorio::game::Chunk_tile_layer ctl{};
		ctl.multi_tile_index = 3;
		ctl.set_multi_tile_parent(&parent);

		// Copy construct
		{
			jactorio::game::Chunk_tile_layer ctl_copy{ctl};
			EXPECT_NE(ctl_copy.multi_tile_index, 0);

			ctl_copy.multi_tile_index = 0;  // Set index to 0 so .is_multi_tile() does not assert false
			EXPECT_FALSE(ctl_copy.is_multi_tile());  // Not considered multi tile since it does not point to parent
		}
		// Copy assignment
		{
			jactorio::game::Chunk_tile_layer ctl_copy = ctl;
			EXPECT_NE(ctl_copy.multi_tile_index, 0);

			ctl_copy.multi_tile_index = 0;  // Set index to 0 so .is_multi_tile() does not assert false
			EXPECT_FALSE(ctl_copy.is_multi_tile());  // Not considered multi tile since it does not point to parent
		}
	}

	TEST(ChunkTileLayer, CopyMultiTileTopLeft) {
		jactorio::game::Chunk_tile_layer ctl{};
		ctl.multi_tile_index = 0;
		ctl.init_multi_tile_prop(1, 2);

		// Copy construct
		{
			const jactorio::game::Chunk_tile_layer ctl_copy{ctl};
			EXPECT_NE(&ctl_copy.get_multi_tile_data(), &ctl.get_multi_tile_data());
		}
		// Copy assignment
		{
			const jactorio::game::Chunk_tile_layer ctl_copy = ctl;
			EXPECT_NE(&ctl_copy.get_multi_tile_data(), &ctl.get_multi_tile_data());
		}
	}

	TEST(ChunkTileLayer, IsMultiTile) {
		{
			jactorio::game::Chunk_tile_layer ctl{};
			ctl.multi_tile_index = 0;
			EXPECT_FALSE(ctl.is_multi_tile());
		}

		{
			jactorio::game::Chunk_tile_layer ctl{};
			ctl.multi_tile_index = 0;
			ctl.init_multi_tile_prop(1, 2);
			EXPECT_TRUE(ctl.is_multi_tile());
		}
		{
			jactorio::game::Chunk_tile_layer ctl{};
			ctl.multi_tile_index = 4;
			ctl.set_multi_tile_parent(&ctl);  // Some random address
			EXPECT_TRUE(ctl.is_multi_tile());
		}
	}

	TEST(ChunkTileLayer, IsMultiTileTopLeft) {
		{
			jactorio::game::Chunk_tile_layer ctl{};
			ctl.multi_tile_index = 0;
			EXPECT_FALSE(ctl.is_multi_tile_top_left());
		}

		{
			jactorio::game::Chunk_tile_layer ctl{};
			ctl.multi_tile_index = 0;
			ctl.init_multi_tile_prop(1, 2);
			EXPECT_TRUE(ctl.is_multi_tile_top_left());
		}
		{
			jactorio::game::Chunk_tile_layer ctl{};
			ctl.multi_tile_index = 4;
			ctl.set_multi_tile_parent(&ctl);  // Some random address
			EXPECT_FALSE(ctl.is_multi_tile_top_left());
		}
	}


	TEST(ChunkTileLayer, GetMultiTileTopLeft) {
		jactorio::game::Chunk_tile_layer first{};
		first.init_multi_tile_prop(12, 32);

		jactorio::game::Chunk_tile_layer second{};
		second.multi_tile_index = 1;
		second.set_multi_tile_parent(&first);


		EXPECT_EQ(first.get_multi_tile_top_left(), &first);
		EXPECT_EQ(second.get_multi_tile_top_left(), &first);
	}

	TEST(ChunkTileLayer, SetMultiTileParent) {
		jactorio::game::Chunk_tile_layer first{};
		jactorio::game::Chunk_tile_layer second{};
		second.multi_tile_index = 1;
		second.set_multi_tile_parent(&first);

		EXPECT_EQ(second.get_multi_tile_parent(), &first);
	}

	TEST(ChunkTileLayer, OverrideMultiTileData) {
		// Both should return the same multi tile data
		jactorio::game::Chunk_tile_layer first{};
		first.init_multi_tile_prop(12, 32);
		first.init_multi_tile_prop(3, 30);

		jactorio::game::Multi_tile_data& data = first.get_multi_tile_data();

		EXPECT_EQ(data.multi_tile_span, 3);
		EXPECT_EQ(data.multi_tile_height, 30);
	}

	TEST(ChunkTileLayer, GetMultiTileData) {
		// Both should return the same multi tile data
		jactorio::game::Chunk_tile_layer first{};
		first.init_multi_tile_prop(12, 32);

		jactorio::game::Chunk_tile_layer second{};
		second.multi_tile_index = 1;
		second.set_multi_tile_parent(&first);


		jactorio::game::Multi_tile_data& data_1 = first.get_multi_tile_data();
		jactorio::game::Multi_tile_data& data_2 = second.get_multi_tile_data();

		EXPECT_EQ(&data_1, &data_2);
		EXPECT_NE(&data_1, nullptr);
	}


	TEST(ChunkTileLayer, GetOffsetX) {
		jactorio::game::Chunk_tile_layer parent{};
		parent.init_multi_tile_prop(10, 2);

		jactorio::game::Chunk_tile_layer ctl{};
		ctl.multi_tile_index = 19;
		ctl.set_multi_tile_parent(&parent);

		EXPECT_EQ(ctl.get_offset_x(), 9);
	}

	TEST(ChunkTileLayer, GetOffsetY) {
		jactorio::game::Chunk_tile_layer parent{};
		parent.init_multi_tile_prop(5, 2);

		jactorio::game::Chunk_tile_layer ctl{};
		ctl.multi_tile_index = 20;
		ctl.set_multi_tile_parent(&parent);

		EXPECT_EQ(ctl.get_offset_y(), 4);
	}
}
