//
// Created on 2/22/2020.
// This file is subject to the terms and conditions defined in 'LICENSE' included in the source code package
//

#ifndef JACTORIO_INCLUDE_GAME_WORLD_CHUNK_STRUCT_LAYER_H
#define JACTORIO_INCLUDE_GAME_WORLD_CHUNK_STRUCT_LAYER_H

namespace jactorio::game
{
	/**
	* For a Logic_chunk, stores chunk structures (transport line segments (Not the entities), etc)
	*/
	struct Chunk_struct_layer : Chunk_layer
	{
		Chunk_struct_layer() = default;

		explicit Chunk_struct_layer(data::Prototype_base* proto)
			: Chunk_layer(proto) {
		}

		explicit Chunk_struct_layer(data::Prototype_base* proto,
									const float position_x, const float position_y)
			: Chunk_layer(proto), position_x(position_x), position_y(position_y) {
		}

		// Distance (tiles) from top left of chunk to top left of object
		float position_x = 0;
		float position_y = 0;
	};
}


#endif //JACTORIO_INCLUDE_GAME_WORLD_CHUNK_STRUCT_LAYER_H
