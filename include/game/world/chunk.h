// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_WORLD_CHUNK_H
#define JACTORIO_INCLUDE_GAME_WORLD_CHUNK_H
#pragma once

#include "jactorio.h"

#include <utility>
#include <vector>

#include "game/world/chunk_object_layer.h"
#include "game/world/chunk_tile.h"

namespace jactorio::game
{
	///
	/// \brief A chunk within the game <br>
	/// Made up of tiles and objects:
	///		tiles: Has 32 x 32, fixed grid location
	///		objects: Has no set amount, can exist anywhere on chunk
	class Chunk
	{
	public:
		using ObjectLayerType = std::vector<ChunkObjectLayer>;
		using LogicGroupType = std::vector<ChunkTileLayer*>;

		using ChunkCoord = int32_t;  // Chunk coordinates
		using ChunkPair = std::pair<ChunkCoord, ChunkCoord>;

		static constexpr uint8_t kChunkWidth = 32;
		static constexpr uint16_t kChunkArea = static_cast<uint16_t>(kChunkWidth) * kChunkWidth;


		///
		/// \brief Default initialization of chunk tiles
		/// \param chunk_x X position of chunk
		/// \param chunk_y Y position of chunk
		Chunk(ChunkCoord chunk_x, ChunkCoord chunk_y);

		///
		/// \param chunk_x X position of chunk
		/// \param chunk_y Y position of chunk
		/// \param tiles Array of size 32 * 32 (1024) tiles <br>
	    ///			Do not delete the provided pointer, it will be automatically
		///			freed when Chunk is destructed
		Chunk(ChunkCoord chunk_x, ChunkCoord chunk_y, ChunkTile* tiles);

		~Chunk();

		Chunk(const Chunk& other);
		Chunk(Chunk&& other) noexcept;

		Chunk& operator=(Chunk other);

		friend void swap(Chunk& lhs, Chunk& rhs) noexcept {
			using std::swap;
			swap(lhs.position_, rhs.position_);
			swap(lhs.tiles_, rhs.tiles_);
		}


		J_NODISCARD std::pair<int, int> GetPosition() const { return position_; }

		// ======================================================================
		// Tiles
		J_NODISCARD ChunkTile* Tiles() const {
			return tiles_;
		}

		// ======================================================================
		// Objects - Rendered without being fixed to a tile position
		enum class ObjectLayer
		{
			debug_overlay = 0,  // data::Sprite
			count_
		};

		static constexpr int kObjectLayerCount = static_cast<int>(ObjectLayer::count_);
		ObjectLayerType objects[kObjectLayerCount];

		ObjectLayerType& GetObject(const ObjectLayer layer) {
			return const_cast<ObjectLayerType&>(
				static_cast<const Chunk*>(this)->GetObject(layer)
			);
		}

		J_NODISCARD const ObjectLayerType& GetObject(ObjectLayer layer) const {
			return objects[static_cast<int>(layer)];
		}


		// ======================================================================
		// Items requiring logic updates
		enum class LogicGroup
		{
			transport_line = 0,
			inserter,
			count_
		};

		static constexpr int kLogicGroupCount = static_cast<int>(LogicGroup::count_);

		/// Holds pointer to UniqueData at tile requiring logic update
		LogicGroupType logicGroups[kLogicGroupCount];

		J_NODISCARD LogicGroupType& GetLogicGroup(const LogicGroup layer) {
			return const_cast<LogicGroupType&>(
				static_cast<const Chunk*>(this)->GetLogicGroup(layer)
			);
		}

		J_NODISCARD const LogicGroupType& GetLogicGroup(const LogicGroup layer) const {
			return logicGroups[static_cast<int>(layer)];
		}

	private:
		ChunkPair position_;
		/// Pointers to the actual tiles since they are static size of 32x32
		ChunkTile* tiles_ = nullptr;
	};
}

#endif //JACTORIO_INCLUDE_GAME_WORLD_CHUNK_H
