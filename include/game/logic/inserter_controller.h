// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 05/25/2020

#ifndef JACTORIO_GAME_LOGIC_INSERTER_CONTROLLER_H
#define JACTORIO_GAME_LOGIC_INSERTER_CONTROLLER_H
#pragma once

#include "core/float_math.h"
#include "game/world/world_data.h"

namespace jactorio::game
{
	// Inserter behavior
	//
	// Transport lines:
	//     Take from near side, place on near side
	//     Take from anywhere within 1 tile on the transport line
	//     Put in center of output transport line

	constexpr auto kInserterCenterOffset = 0.5;
	/// Distance from inserter arm radius to the end of the last tile 
	constexpr auto kInserterArmTileGap = 0.3;

	constexpr auto kMinInserterDegree = 0;
	constexpr auto kMaxInserterDegree = 180;

	///
	/// \brief Gets the tile distance of the inserter arm to its resting position at 0 or 180 degrees
	///
	///   / |
	///  /  | x
	/// /   |
	/// ----------- 0 or 180
	///
	/// \param degree Current degree of inserter arm 0 <= degree <= 180
	/// \param target_distance Tiles to the location which the inserter picks up / drops items
	double GetInserterArmOffset(core::TIntDegree degree, uint8_t target_distance);

	///
	/// \brief Gets the tile length of the inserter arm 
	///
	///   / |
	/// x/  |
	/// /   |
	/// ----------- 0 or 180
	///
	/// \param degree Current degree of inserter arm 0 <= degree <= 180
	/// \param target_distance Tiles to the location which the inserter picks up / drops items
	double GetInserterArmLength(core::TIntDegree degree, uint8_t target_distance);


	///
	/// \brief Updates inserter logic for a logic chunk
	void InserterLogicUpdate(WorldData& world_data);
}

#endif // JACTORIO_GAME_LOGIC_INSERTER_CONTROLLER_H
