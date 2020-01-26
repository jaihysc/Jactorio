#ifndef GAME_WORLD_WORLD_GENERATOR_H
#define GAME_WORLD_WORLD_GENERATOR_H

#include "jactorio.h"

namespace jactorio::game::world_generator
{
	// Seed
	void set_world_generator_seed(int seed);
	
	J_NODISCARD int get_world_generator_seed();

	
	/**
	 * Queues a chunk to be generated at specified position, a chunk will be generated on every logic
	 * loop tick
	 */
	void queue_chunk_generation(int chunk_x, const int chunk_y);

	/**
	 * Call once per logic loop tick to generate one chunk only, this keeps performance constant
	 * when generating large amounts of chunks
	 */
	void gen_chunk(uint8_t amount = 1);
}

#endif // GAME_WORLD_WORLD_GENERATOR_H
