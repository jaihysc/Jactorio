#ifndef GAME_WORLD_CHUNK_H
#define GAME_WORLD_CHUNK_H

#include "jactorio.h"

#include <utility>
#include <vector>

#include "game/world/chunk_tile.h"
#include "game/world/chunk_object_layer.h"

namespace jactorio::game
{
	/**
	 * A chunk within the game <br>
	 * Made up of tiles and objects:
	 *		tiles: Has 32 x 32, fixed grid location
	 *		objects: Has no set amount, can exist anywhere on chunk
	 */
	class Chunk
	{
		std::pair<int, int> position_;
		// Pointers to the actual tiles since they are static size of 32x32
		Chunk_tile* tiles_ = nullptr;
		
	public:
		/**
		 * @param x X position of chunk
		 * @param y Y position of chunk
		 * @param tiles Array of size 32 * 32 (1024) tiles <br>
		 *			provide nullptr to use default tile initialization <br>
		 *			Do not delete the provided pointer, it will be automatically
		 *			freed when Chunk is destructed
		 */
		Chunk(int x, int y, Chunk_tile* tiles);
		~Chunk();

		Chunk(const Chunk& other);
		Chunk(Chunk&& other) noexcept;
		
		Chunk& operator=(const Chunk& other);
		Chunk& operator=(Chunk&& other) noexcept;

		// Tiles

		J_NODISCARD std::pair<int, int> get_position() const;

		J_NODISCARD Chunk_tile* tiles_ptr() const {
			return tiles_;
		}

		// Objects
		enum class object_layer
		{
			item_entity = 0,  // data::Sprite
			tree,
			_count
		};

		std::vector<Chunk_object_layer> objects[static_cast<int>(object_layer::_count)];

		std::vector<Chunk_object_layer>& get_object(object_layer layer) {
			return objects[static_cast<int>(layer)];
		}
	};
}

#endif // GAME_WORLD_CHUNK_H
