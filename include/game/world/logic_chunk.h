// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#ifndef JACTORIO_INCLUDE_GAME_WORLD_LOGIC_CHUNK_H
#define JACTORIO_INCLUDE_GAME_WORLD_LOGIC_CHUNK_H
#pragma once

#include "game/world/chunk.h"
#include "game/world/chunk_struct_layer.h"

namespace jactorio::game
{
	///
	/// \brief Contains additional data for chunks that require logic updates <br>
	/// To be stored alongside a game::Chunk
	class LogicChunk
	{
	public:
		explicit LogicChunk(Chunk* chunk)
			: chunk(chunk) {
			assert(chunk != nullptr);  // Logic chunks should always be associated with a chunk
		}

		// The chunk which this logic chunk is associated with
		Chunk* const chunk;


		// Structures - Objects but not rendered with no coordinate attached
		enum class StructLayer
		{
			transport_line = 0,  // game::Transport_line_segment
			count_
		};

		static constexpr int kStructLayerCount = static_cast<int>(StructLayer::count_);

		std::vector<ChunkStructLayer> structs[kStructLayerCount];

		J_NODISCARD std::vector<ChunkStructLayer>& GetStruct(const StructLayer layer) {
			return const_cast<std::vector<ChunkStructLayer>&>(
				static_cast<const LogicChunk*>(this)->GetStruct(layer)
			);
		}

		J_NODISCARD const std::vector<ChunkStructLayer>& GetStruct(StructLayer layer) const {
			return structs[static_cast<int>(layer)];
		}
	};
}

#endif //JACTORIO_INCLUDE_GAME_WORLD_LOGIC_CHUNK_H
