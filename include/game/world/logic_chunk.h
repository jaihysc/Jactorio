// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#ifndef JACTORIO_INCLUDE_GAME_WORLD_LOGIC_CHUNK_H
#define JACTORIO_INCLUDE_GAME_WORLD_LOGIC_CHUNK_H
#pragma once

#include "game/world/chunk.h"
#include "game/world/chunk_struct_layer.h"

namespace jactorio::game
{
	/**
	 * Contains additional data for chunks that require logic updates <br>
	 * To be stored alongside a game::Chunk
	 */
	class Logic_chunk
	{
	public:
		explicit Logic_chunk(Chunk* chunk)
			: chunk(chunk) {
			assert(chunk != nullptr);  // Logic chunks should always be associated with a chunk
		}

		// The chunk which this logic chunk is associated with
		Chunk* const chunk;


		// Structures - Objects but not rendered with no coordinate attached
		enum class structLayer
		{
			transport_line = 0,  // game::Transport_line_segment
			count_
		};

		static constexpr int struct_layer_count = static_cast<int>(structLayer::count_);

		std::vector<Chunk_struct_layer> structs[struct_layer_count];

		J_NODISCARD std::vector<Chunk_struct_layer>& get_struct(structLayer layer) {
			return structs[static_cast<int>(layer)];
		}

		J_NODISCARD const std::vector<Chunk_struct_layer>& get_struct_read_only(structLayer layer) const {
			return structs[static_cast<int>(layer)];
		}
	};
}

#endif //JACTORIO_INCLUDE_GAME_WORLD_LOGIC_CHUNK_H
