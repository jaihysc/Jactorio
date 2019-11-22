#ifndef GAME_WORLD_WORLD_GENERATOR_H
#define GAME_WORLD_WORLD_GENERATOR_H

#include <atomic>

namespace jactorio::game::world_generator
{
	/**
	 * Generates a chunk and adds it to the world when done <br>
	 * Call this with a std::thread to to this in async
	 * @param chunk_x X position of chunk to generate
	 * @param chunk_y Y position of chunk to generate
	 * @param thread_counter This variable will be decremented once the thread has finished
	 */
	void generate_chunk(int chunk_x, int chunk_y, std::atomic<int>* thread_counter);
}

#endif // GAME_WORLD_WORLD_GENERATOR_H
