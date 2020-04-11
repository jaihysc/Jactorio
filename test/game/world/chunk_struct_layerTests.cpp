// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#include <gtest/gtest.h>

#include "game/world/chunk_struct_layer.h"

namespace game
{
	TEST(chunk_struct_layer, to_position) {
		using namespace jactorio::game;

		EXPECT_FLOAT_EQ(Chunk_struct_layer::to_position(0, 10), 10.f);
		EXPECT_FLOAT_EQ(Chunk_struct_layer::to_position(2, 64), 0.f);

		EXPECT_FLOAT_EQ(Chunk_struct_layer::to_position(-1, -32), 0.f);
		EXPECT_FLOAT_EQ(Chunk_struct_layer::to_position(-1, -1), 31.f);
	}
}
