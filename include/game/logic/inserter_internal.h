// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 05/18/2020

#ifndef JACTORIO_GAME_LOGIC_INSERTER_INTERNAL_H
#define JACTORIO_GAME_LOGIC_INSERTER_INTERNAL_H
#pragma once

#include "data/prototype/entity/inserter.h"

namespace jactorio::game
{
	///
	/// \brief Holds the internal structure for inserters
	struct InserterInternal final : UniqueDataStruct
	{
		using RotationDegree = data::Inserter::RotationDegree;

		///
		/// \brief Rotation degree of current inserter, from standard position
		RotationDegree rotationDegree;
	};
}

#endif // JACTORIO_GAME_LOGIC_INSERTER_INTERNAL_H
