// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/world/chunk.h"

using namespace jactorio;

game::Chunk::Chunk(const Chunk& other)
    : overlays{other.overlays}, logicGroups{other.logicGroups}, position_{other.position_}, layers_{other.layers_} {

    ResolveLogicEntries(other);
}

game::Chunk::Chunk(Chunk&& other) noexcept
    : overlays{std::move(other.overlays)},
      logicGroups{std::move(other.logicGroups)},
      position_{other.position_},
      layers_{std::move(other.layers_)} {

    ResolveLogicEntries(other);
}

// ======================================================================

game::Chunk::OverlayContainerT& game::Chunk::GetOverlay(const OverlayLayer layer) {
    return const_cast<OverlayContainerT&>(static_cast<const Chunk*>(this)->GetOverlay(layer));
}

const game::Chunk::OverlayContainerT& game::Chunk::GetOverlay(OverlayLayer layer) const {
    return overlays[static_cast<OverlayArrayT::size_type>(layer)];
}


game::Chunk::LogicGroupContainerT& game::Chunk::GetLogicGroup(const LogicGroup l_group) {
    return const_cast<LogicGroupContainerT&>(static_cast<const Chunk*>(this)->GetLogicGroup(l_group));
}

const game::Chunk::LogicGroupContainerT& game::Chunk::GetLogicGroup(const LogicGroup l_group) const {
    return logicGroups[static_cast<LogicGroupArrayT::size_type>(l_group)];
}

// ======================================================================

void game::Chunk::ResolveLogicEntries(const Chunk& other) noexcept {
    for (int i_group = 0; i_group < kLogicGroupCount; ++i_group) {
        for (auto& entry : logicGroups[i_group]) {

            // entry currently holds pointer info for other chunk
            auto info = other.GetTileInfo(*entry);

            entry = &GetCTile(info.coord, info.tileLayer);
        }
    }
}


game::Chunk::TileInfo game::Chunk::GetTileInfo(const ChunkTile& tile) const noexcept {
    for (uint8_t y = 0; y < kChunkWidth; ++y) {
        for (uint8_t x = 0; x < kChunkWidth; ++x) {

            for (uint8_t i = 0; i < kTileLayerCount; ++i) {
                const auto tlayer = static_cast<TileLayer>(i);

                const auto& i_tile = GetCTile({x, y}, tlayer);
                if (&i_tile == &tile) {
                    return TileInfo{{x, y}, tlayer};
                }
            }
        }
    }

    assert(false);
    return TileInfo{{}, TileLayer::base};
}

game::Chunk::SerialLogicGroupArrayT game::Chunk::ToSerializeLogicGroupArray() const {
    SerialLogicGroupArrayT serial_logic;

    for (int i = 0; i < kLogicGroupCount; ++i) {
        serial_logic[i].reserve(logicGroups[i].size());
    }


    // Convert tile* to world position + layer index
    for (int i_group = 0; i_group < kLogicGroupCount; ++i_group) {
        auto& serial_group = serial_logic[i_group];

        for (const auto* tile : logicGroups[i_group]) {
            assert(tile != nullptr);
            serial_group.push_back(GetTileInfo(*tile));
        }
    }

    return serial_logic;
}

void game::Chunk::FromSerializeLogicGroupArray(const SerialLogicGroupArrayT& serial_logic) {
    assert(serial_logic.size() == kLogicGroupCount);

    for (int i_group = 0; i_group < kLogicGroupCount; ++i_group) {
        const auto& serial_group = serial_logic[i_group];

        for (const auto& serial_entry : serial_group) {

            auto& tile = GetCTile(serial_entry.coord, serial_entry.tileLayer);
            logicGroups[i_group].push_back(&tile);
        }

        assert(logicGroups[i_group].size() == serial_group.size());
    }
}
