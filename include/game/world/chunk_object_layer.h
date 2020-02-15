#ifndef GAME_WORLD_CHUNK_OBJECT_LAYER_H
#define GAME_WORLD_CHUNK_OBJECT_LAYER_H

#include "game/world/chunk_layer.h"

namespace jactorio::game
{
	/**
	 * Similar to a Chunk_tile_layer, but for objects!
	 */
	struct Chunk_object_layer : Chunk_layer
	{
		Chunk_object_layer() = default;

		explicit Chunk_object_layer(data::Prototype_base* proto)
			: Chunk_layer(proto) {
		}

		explicit Chunk_object_layer(data::Prototype_base* proto,
		                            const float position_x, const float position_y,
		                            const float size_x, const float size_y)
			: Chunk_layer(proto),
			  position_x(position_x), position_y(position_y), size_x(size_x), size_y(size_y) {
		}

		// Distance (tiles) from top left of chunk to top left of object
		float position_x = 0;
		float position_y = 0;

		float size_x = 0;
		float size_y = 0;
	};
}

#endif // GAME_WORLD_CHUNK_OBJECT_LAYER_H