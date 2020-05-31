// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 05/25/2020

#include "game/logic/inserter_controller.h"

#include <cmath>

#include "data/prototype/entity/inserter.h"

double jactorio::game::GetInserterArmOffset(const core::TIntDegree degree, const uint8_t target_distance) {
	const auto result = (kInserterCenterOffset + target_distance - kInserterArmTileGap) * core::TanF(degree);
	return fabs(result);
}

void InserterUpdate(jactorio::game::DeferralTimer& deferral_timer,
                    const jactorio::data::Inserter& inserter_proto, jactorio::data::InserterData& inserter_data) {
	using namespace jactorio::game;
	using namespace jactorio;

	switch (inserter_data.status) {

	case data::InserterData::Status::dropoff:
		return;
	case data::InserterData::Status::pickup:
		if (inserter_data.pickup.Pickup(inserter_data.rotationDegree, 1,
		                                inserter_data.heldItem)) {
			// Picked up item
			auto elapse_tick = static_cast<double>(kMaxInserterDegree - kMinInserterDegree);
			elapse_tick /= inserter_proto.rotationSpeed.getAsDouble();

			// Always round up for ticks to reach dropoff
			double fraction;
			if (modf(elapse_tick, &fraction) != 0) {
				elapse_tick++;
			}

			inserter_data.status = data::InserterData::Status::dropoff;
			deferral_timer.RegisterFromTick(inserter_proto, &inserter_data, static_cast<GameTickT>(elapse_tick));
		}
		return;

	default:
		assert(false);
	}
}

void jactorio::game::InserterLogicUpdate(WorldData& world_data) {
	for (auto* chunk : world_data.LogicGetChunks()) {
		for (auto* tile_layer : chunk->GetLogicGroup(Chunk::LogicGroup::inserter)) {
			auto* inserter_data = tile_layer->GetUniqueData<data::InserterData>();
			assert(inserter_data);

			const auto* proto_data = tile_layer->GetPrototypeData<data::Inserter>();
			assert(proto_data);

			InserterUpdate(world_data.deferralTimer,
			               *proto_data, *inserter_data);
		}
	}
}
