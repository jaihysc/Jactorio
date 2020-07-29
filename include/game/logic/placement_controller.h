// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_LOGIC_PLACEMENT_CONTROLLER_H
#define JACTORIO_INCLUDE_GAME_LOGIC_PLACEMENT_CONTROLLER_H
#pragma once

#include "data/prototype/entity/entity.h"
#include "game/world/chunk_tile.h"
#include "game/world/world_data.h"

namespace jactorio::game
{
	// ChunkTileLayer data layout
	//
	// ^ * *
	// * * *
	//
	// For 3 x 2 placement, prototype data is stored at each tile       ^ and *
	//                      unique data is stored at the Top Left tile  ^

	///
	/// \return true if a entity with the specified dimensions can be placed at x, y
	bool PlacementLocationValid(WorldData& world_data,
	                            uint8_t tile_width, uint8_t tile_height,
	                            WorldData::WorldCoord x, WorldData::WorldCoord y);

	// ======================================================================

	///
	/// \brief Places / removes entities at world coordinates
	/// \remark Pass nullptr for entity to remove
	/// \return true if placed / removed successfully
	bool PlaceEntityAtCoords(WorldData& world_data, const data::Entity* entity,
	                         WorldData::WorldCoord x, WorldData::WorldCoord y);

	bool PlaceEntityAtCoords(WorldData& world_data, const data::Entity* entity,
	                         const WorldData::WorldPair& world_pair);
}

#endif //JACTORIO_INCLUDE_GAME_LOGIC_PLACEMENT_CONTROLLER_H
