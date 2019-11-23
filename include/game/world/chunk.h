#ifndef GAME_WORLD_CHUNK_H
#define GAME_WORLD_CHUNK_H

#include <utility>

#include "game/world/tile.h"

namespace jactorio::game
{
	class Chunk
	{
		std::pair<int, int> position_;
		// Pointers to the actual tiles since they are static
		Chunk_tile* tiles_ [32 * 32];
		
	public:
		/**
		 * @param x X position of chunk
		 * @param y Y position of chunk
		 * @param tiles Array of size 32 * 32 (1024) tiles <br>
		 *			provide nullptr to use not initialize tiles
		 */
		Chunk(int x, int y, Chunk_tile* tiles);
		~Chunk();

		[[nodiscard]] std::pair<int, int> get_position() const;

		[[nodiscard]] Chunk_tile** tiles_ptr() {
			return tiles_;
		};
	};
}

#endif // GAME_WORLD_CHUNK_H
