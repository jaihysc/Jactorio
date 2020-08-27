// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/world/chunk.h"

#include <limits>

using namespace jactorio;

game::Chunk::Chunk(const Chunk& other)
	: overlays{other.overlays},
	  logicGroups{other.logicGroups},
	  position_{other.position_},
	  tiles_{other.tiles_} {

	ResolveLogicEntries(other);
}

game::Chunk::Chunk(Chunk&& other) noexcept
	: overlays{std::move(other.overlays)},
	  logicGroups{std::move(other.logicGroups)},
	  position_{other.position_},
	  tiles_{std::move(other.tiles_)} {

	ResolveLogicEntries(other);
}

// ======================================================================

game::Chunk::TileArrayT& game::Chunk::Tiles() {
	return tiles_;
}

const game::Chunk::TileArrayT& game::Chunk::Tiles() const {
	return tiles_;
}


game::ChunkTile& game::Chunk::GetCTile(const ChunkTileCoordAxis x, const ChunkTileCoordAxis y) {
	assert(x < kChunkWidth);
	assert(y < kChunkWidth);

	using IndexT = uint16_t;

	static_assert(std::numeric_limits<IndexT>::max() > kChunkArea);
	return tiles_[core::SafeCast<IndexT>(y) * kChunkWidth + x];
}

const game::ChunkTile& game::Chunk::GetCTile(const ChunkTileCoordAxis x, const ChunkTileCoordAxis y) const {
	return const_cast<Chunk*>(this)->GetCTile(x, y);
}

game::ChunkTile& game::Chunk::GetCTile(const ChunkTileCoord& coord) {
	return GetCTile(coord.x, coord.y);
}

const game::ChunkTile& game::Chunk::GetCTile(const ChunkTileCoord& coord) const {
	return GetCTile(coord.x, coord.y);
}


game::Chunk::OverlayContainerT& game::Chunk::GetOverlay(const OverlayLayer layer) {
	return const_cast<OverlayContainerT&>(
		static_cast<const Chunk*>(this)->GetOverlay(layer)
	);
}

const game::Chunk::OverlayContainerT& game::Chunk::GetOverlay(OverlayLayer layer) const {
	return overlays[static_cast<OverlayArrayT::size_type>(layer)];
}


game::Chunk::LogicGroupContainerT& game::Chunk::GetLogicGroup(const LogicGroup layer) {
	return const_cast<LogicGroupContainerT&>(
		static_cast<const Chunk*>(this)->GetLogicGroup(layer)
	);
}

const game::Chunk::LogicGroupContainerT& game::Chunk::GetLogicGroup(const LogicGroup layer) const {
	return logicGroups[static_cast<LogicGroupArrayT::size_type>(layer)];
}

// ======================================================================

void game::Chunk::ResolveLogicEntries(const Chunk& other) noexcept {
	for (int i_group = 0; i_group < kLogicGroupCount; ++i_group) {
		for (auto& entry : logicGroups[i_group]) {

			// entry currently holds pointer info for other chunk
			auto info = other.GetLayerInfo(*entry);

			entry = &GetCTile(info.coord).GetLayer(info.tileLayer);
		}
	}
}


game::Chunk::TileLayerInfo game::Chunk::GetLayerInfo(const ChunkTileLayer& ctl) const noexcept {
	for (uint8_t y = 0; y < kChunkWidth; ++y) {
		for (uint8_t x = 0; x < kChunkWidth; ++x) {
			const auto& tile = GetCTile(x, y);

			for (uint8_t i = 0; i < ChunkTile::kTileLayerCount; ++i) {
				const auto& layer = tile.GetLayer(i);
				if (&layer == &ctl) {
					return TileLayerInfo{{x, y}, static_cast<TileLayer>(i)};
				}
			}

		}
	}

	assert(false);
	return TileLayerInfo{{}, TileLayer::base};
}

game::Chunk::SerialLogicGroupArrayT game::Chunk::ToSerializeLogicGroupArray() const {
	SerialLogicGroupArrayT serial_logic;

	for (int i = 0; i < kLogicGroupCount; ++i) {
		serial_logic[i].reserve(logicGroups[i].size());
	}


	// Convert pointers to tile layers to world position + layer index
	for (int i_group = 0; i_group < kLogicGroupCount; ++i_group) {
		auto& serial_group = serial_logic[i_group];

		for (const auto* logic_tile_layer : logicGroups[i_group]) {
			serial_group.push_back(GetLayerInfo(*logic_tile_layer));
		}
	}

	return serial_logic;
}

void game::Chunk::FromSerializeLogicGroupArray(const SerialLogicGroupArrayT& serial_logic) {
	assert(serial_logic.size() == kLogicGroupCount);

	for (int i_group = 0; i_group < kLogicGroupCount; ++i_group) {
		const auto& serial_group = serial_logic[i_group];

		for (const auto& serial_entry : serial_group) {

			auto& layer = GetCTile(serial_entry.coord).GetLayer(serial_entry.tileLayer);
			logicGroups[i_group].push_back(&layer);
		}

		assert(logicGroups[i_group].size() == serial_group.size());
	}
}
