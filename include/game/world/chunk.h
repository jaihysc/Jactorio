// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_WORLD_CHUNK_H
#define JACTORIO_INCLUDE_GAME_WORLD_CHUNK_H
#pragma once

#include "jactorio.h"

#include <array>
#include <vector>

#include "data/cereal/serialize.h"
#include "game/world/chunk_tile.h"
#include "game/world/overlay_element.h"

#include <cereal/types/array.hpp>
#include <cereal/types/vector.hpp>

namespace jactorio::game
{
	///
	/// \brief A chunk within the game
	///
	/// Made up of tiles and objects:
	///		tiles: Has 32 x 32, fixed grid location
	///		objects: Has no set amount, can exist anywhere on chunk
	class Chunk
	{
	public:
		using OverlayContainerT = std::vector<OverlayElement>;
		using LogicGroupContainerT = std::vector<ChunkTileLayer*>;


		static constexpr uint8_t kChunkWidth = 32;
		static constexpr uint16_t kChunkArea = static_cast<uint16_t>(kChunkWidth) * kChunkWidth;

		enum class LogicGroup
		{
			transport_line = 0,
			inserter,
			count_
		};

		static constexpr int kLogicGroupCount = static_cast<int>(LogicGroup::count_);

	private:
		using TileArrayT = std::array<ChunkTile, kChunkArea>;
		using OverlayArrayT = std::array<OverlayContainerT, kOverlayLayerCount>;
		using LogicGroupArrayT = std::array<LogicGroupContainerT, kLogicGroupCount>;

	public:
		///
		/// \remark For cereal deserialization only
		Chunk() = default;
		
		///
		/// \brief Default initialization of chunk tiles
		Chunk(const ChunkCoordAxis chunk_x, const ChunkCoordAxis chunk_y)
			: position_({chunk_x, chunk_y}) {
		}


		// ======================================================================

		J_NODISCARD ChunkCoord GetPosition() const { return position_; }

		J_NODISCARD TileArrayT& Tiles() {
			return tiles_;
		}

		J_NODISCARD const TileArrayT& Tiles() const {
			return tiles_;
		}


		// Overlays - Rendered without being fixed to a tile position

		OverlayContainerT& GetOverlay(const OverlayLayer layer) {
			return const_cast<OverlayContainerT&>(
				static_cast<const Chunk*>(this)->GetOverlay(layer)
			);
		}

		J_NODISCARD const OverlayContainerT& GetOverlay(OverlayLayer layer) const {
			return overlays[static_cast<OverlayArrayT::size_type>(layer)];
		}

		
		// Items requiring logic updates

		J_NODISCARD LogicGroupContainerT& GetLogicGroup(const LogicGroup layer) {
			return const_cast<LogicGroupContainerT&>(
				static_cast<const Chunk*>(this)->GetLogicGroup(layer)
			);
		}

		J_NODISCARD const LogicGroupContainerT& GetLogicGroup(const LogicGroup layer) const {
			return logicGroups[static_cast<LogicGroupArrayT::size_type>(layer)];
		}


		CEREAL_SERIALIZE(archiver) {
			archiver(position_, tiles_); // TODO incomplete
		}

		OverlayArrayT overlays;

		/// Holds pointer to UniqueData at tile requiring logic update
		LogicGroupArrayT logicGroups;

	private:
		ChunkCoord position_;
		TileArrayT tiles_;
	};
}

#endif //JACTORIO_INCLUDE_GAME_WORLD_CHUNK_H
