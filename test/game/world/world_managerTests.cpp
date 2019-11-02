#include <gtest/gtest.h>

#include "game/world/chunk.h"
#include "game/world/world_manager.h"

TEST(world_manager, world_add_chunk) {
	// Chunks initialized with empty tiles
	const auto chunk = new jactorio::game::Chunk{5, 1, nullptr};

	// Returns pointer to chunk which was added
	const auto added_chunk = jactorio::game::world_manager::add_chunk(chunk);
	
	// Chunk knows its own location
	EXPECT_EQ(added_chunk->get_position().first, 5);
	EXPECT_EQ(added_chunk->get_position().second, 1);

	// Chunk gives its contents in a const array pointer, size 32 * 32
	const auto chunk_tiles_ptr = added_chunk->tiles_ptr();
	for (int i = 0; i < 32 * 32; ++i) {
		// Blank Tile id when the tiles are uninitialized
		EXPECT_EQ(chunk_tiles_ptr[i]->texture_iname, "!");
	}

	jactorio::game::world_manager::clear_chunk_data();
}

TEST(world_manager, world_add_chunk_override) {
	// Chunks initialized with empty tiles
	const auto chunk = new jactorio::game::Chunk{ 5, 1, nullptr };
	const auto chunk2 = new jactorio::game::Chunk{ 10, 0, nullptr };

	// Adding a chunk to an existing location overwrites it
	const auto added_chunk = jactorio::game::world_manager::add_chunk(chunk);
	const auto added_chunk2 = jactorio::game::world_manager::add_chunk(chunk2);

	// Retrieved chunk should be chunk2
	EXPECT_NE(added_chunk, added_chunk2);
	
	EXPECT_EQ(added_chunk, chunk);
	EXPECT_EQ(added_chunk2, chunk2);

	jactorio::game::world_manager::clear_chunk_data();
}

TEST(world_manager, world_get_chunk) {
	const auto chunk = new jactorio::game::Chunk{ 5, 1, nullptr };
	const auto* added_chunk = jactorio::game::world_manager::add_chunk(chunk);
	
	EXPECT_EQ(jactorio::game::world_manager::get_chunk(0, 0), nullptr);
	EXPECT_EQ(jactorio::game::world_manager::get_chunk(5, 1), added_chunk);

	jactorio::game::world_manager::clear_chunk_data();
}


TEST(world_manager, clear_chunk_data) {
	const auto chunk = new jactorio::game::Chunk{ 6, 6, nullptr };
	const auto* added_chunk = jactorio::game::world_manager::add_chunk(chunk);

	EXPECT_EQ(jactorio::game::world_manager::get_chunk(6, 6), added_chunk);

	jactorio::game::world_manager::clear_chunk_data();

	// Chunk no longer exists after it was cleared
	EXPECT_EQ(jactorio::game::world_manager::get_chunk(6, 6), nullptr);
}
