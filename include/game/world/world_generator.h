#ifndef GAME_WORLD_WORLD_GENERATOR_H
#define GAME_WORLD_WORLD_GENERATOR_H

namespace jactorio::game::world_generator
{
	/**
	 * Generates a chunk and adds it to the world when done <br>
	 * Call this with a std::thread to to this in async
	 * @param chunk_x X position of chunk to generate
	 * @param chunk_y Y position of chunk to generate
	 */
	void generate_chunk(int chunk_x, int chunk_y);
}

#endif // GAME_WORLD_WORLD_GENERATOR_H
