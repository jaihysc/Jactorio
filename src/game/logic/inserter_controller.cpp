// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 05/17/2020

#include "game/logic/inserter_controller.h"
#include "data/prototype/entity/inserter.h"
#include "game/logic/inserter_internal.h"

double jactorio::game::GetInserterArmOffset(const core::TIntDegree degree, const uint8_t target_distance) {
	const auto result = (kInserterCenterOffset + target_distance - kInserterArmTileGap) * core::TanF(degree);
	return fabs(result);
}

void jactorio::game::InserterLogicUpdate(WorldData& world_data) {
}
