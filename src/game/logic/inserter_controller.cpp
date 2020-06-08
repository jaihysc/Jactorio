// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 05/25/2020

#include "game/logic/inserter_controller.h"

#include <cmath>

#include "data/prototype/entity/inserter.h"

double jactorio::game::GetInserterArmOffset(const core::TIntDegree degree, const uint8_t target_distance) {
	const auto result = (kInserterCenterOffset + target_distance - kInserterArmTileGap) * core::TanF(degree);
	return fabs(result);
}

double jactorio::game::GetInserterArmLength(const core::TIntDegree degree, const uint8_t target_distance) {
	const auto result = (kInserterCenterOffset + target_distance - kInserterArmTileGap) / core::CosF(degree);
	return fabs(result);
}


void InserterUpdate(const jactorio::data::Inserter& inserter_proto, jactorio::data::InserterData& inserter_data) {
	using namespace jactorio::game;
	using namespace jactorio;

	switch (inserter_data.status) {

	case data::InserterData::Status::dropoff:
		inserter_data.rotationDegree -= inserter_proto.rotationSpeed;
		if (inserter_data.rotationDegree <= data::ToRotationDegree(kMinInserterDegree)) {
			inserter_data.rotationDegree = 0;  // Prevents underflow if the inserter sits idle for a long time

			if (inserter_data.dropoff.DropOff(inserter_data.heldItem)) {
				inserter_data.status = data::InserterData::Status::pickup;
			}
		}

		return;

	case data::InserterData::Status::pickup:
		// Rotate the inserter
		inserter_data.rotationDegree += inserter_proto.rotationSpeed;
		if (inserter_data.rotationDegree > data::ToRotationDegree(kMaxInserterDegree))
			inserter_data.rotationDegree = kMaxInserterDegree;


		if (inserter_data.rotationDegree > data::ToRotationDegree(90.f)) {  // Cos 90 is invalid
			const auto required_arm_length =
				GetInserterArmLength(inserter_data.rotationDegree.getAsInteger(), 1);   // TODO configurable distance

			if (required_arm_length <= inserter_proto.armLength &&
				inserter_data.pickup.Pickup(inserter_data.rotationDegree, 1, inserter_data.heldItem)) {

				inserter_data.status = data::InserterData::Status::dropoff;
			}
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

			InserterUpdate(*proto_data, *inserter_data);
		}
	}
}
