// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_LOGIC_PLACEMENT_CONTROLLER_H
#define JACTORIO_INCLUDE_GAME_LOGIC_PLACEMENT_CONTROLLER_H
#pragma once

#include "core/data_type.h"
#include "core/orientation.h"

namespace jactorio::proto
{
    class Entity;
}

namespace jactorio::game
{
    ///
    /// Determines if entity of given dimensions can be placed at specified coord
    /// \return true if a entity with the specified dimensions can be placed at x, y
    bool PlacementLocationValid(WorldData& world, const WorldCoord& coord, uint8_t tile_width, uint8_t tile_height);

    // ======================================================================

    ///
    /// Places / removes entities at world coordinates
    /// \remark Pass nullptr for entity to remove
    /// \return true if placed / removed successfully
    bool PlaceEntityAtCoords(WorldData& world, const WorldCoord& coord, Orientation orien, const proto::Entity* entity);
} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_LOGIC_PLACEMENT_CONTROLLER_H
