// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "data/prototype/sprite.h"
#include "game/world/chunk.h"
#include "game/world/world_data.h"

namespace jactorio::game
{
	TEST(Chunk, GetObjectLayer) {
		Chunk chunk_a{0, 0};

		// Should return the layer specified by the index of the enum objectLayer
		EXPECT_EQ(&chunk_a.GetOverlay(OverlayLayer::general), &chunk_a.overlays[0]);
	}

	TEST(Chunk, GetLogicGroup) {
		Chunk chunk{0, 0};

		// Should return the layer specified by the index of the enum objectLayer
		EXPECT_EQ(&chunk.GetLogicGroup(Chunk::LogicGroup::transport_line), &chunk.logicGroups[0]);
	}
}
