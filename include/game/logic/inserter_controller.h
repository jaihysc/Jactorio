// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 05/17/2020

#ifndef JACTORIO_GAME_LOGIC_INSERTER_CONTROLLER_H
#define JACTORIO_GAME_LOGIC_INSERTER_CONTROLLER_H
#pragma once

#include "game/world/world_data.h"

namespace jactorio::game
{
	// Inserter behavior
	//
	// Transport lines:
	//     Take from far side, place on far side
	//     Take from anywhere within 1 tile on the transport line
	//     Put in center of output transport line

	///
	/// \brief Updates inserter logic for a logic chunk
	void InserterLogicUpdate(WorldData& world_data);
}

#endif // JACTORIO_GAME_LOGIC_INSERTER_CONTROLLER_H
