// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/logic/inserter_controller.h"

#include "data/prototype/entity/inserter.h"

using namespace jactorio;

double game::GetInserterArmOffset(const core::TIntDegree degree, const uint8_t target_distance) {
	auto result = kInserterCenterOffset + target_distance - kInserterArmTileGap;
	result *= core::TanF(degree);
	result *= -1;
	result += kInserterCenterOffset;

	return result;
}

double game::GetInserterArmLength(const core::TIntDegree degree, const uint8_t target_distance) {
	auto result = kInserterCenterOffset + target_distance - kInserterArmTileGap;
	result /= core::CosF(degree);
	result *= -1;

	return result;
}


void InserterUpdate(game::LogicData& logic_data,
                    const data::Inserter& inserter_proto, data::InserterData& inserter_data) {
	using namespace game;

	assert(inserter_proto.rotationSpeed.getAsDouble() != 0);

	switch (inserter_data.status) {

	case data::InserterData::Status::dropoff:
		inserter_data.rotationDegree -= inserter_proto.rotationSpeed;
		if (inserter_data.rotationDegree <= data::ToRotationDegree(kMinInserterDegree)) {
			inserter_data.rotationDegree = 0;  // Prevents underflow if the inserter sits idle for a long time

			if (inserter_data.dropoff.DropOff(logic_data, inserter_data.heldItem)) {
				inserter_data.status = data::InserterData::Status::pickup;
			}
		}

		return;

	case data::InserterData::Status::pickup:
		// Rotate the inserter
		inserter_data.rotationDegree += inserter_proto.rotationSpeed;

		if (inserter_data.rotationDegree > data::ToRotationDegree(kMaxInserterDegree)) {
			inserter_data.rotationDegree = kMaxInserterDegree;

			constexpr int pickup_amount = 1;

			const auto* to_be_picked_item =
				inserter_data.pickup.GetPickup(logic_data,
				                               inserter_proto.tileReach,
				                               inserter_data.rotationDegree);

			// Do not pick up item if it cannot be dropped off
			if (!inserter_data.dropoff.CanDropOff(logic_data, to_be_picked_item))
				return;


			const auto result = inserter_data.pickup.Pickup(logic_data,
			                                                inserter_proto.tileReach,
			                                                inserter_data.rotationDegree,
			                                                pickup_amount);
			if (result.first) {
				inserter_data.heldItem = result.second;

				inserter_data.status = data::InserterData::Status::dropoff;
			}
		}

		return;

	default:
		assert(false);
	}
}

void game::InserterLogicUpdate(WorldData& world_data, LogicData& logic_data) {
	for (auto* chunk : world_data.LogicGetChunks()) {
		for (auto* tile_layer : chunk->GetLogicGroup(Chunk::LogicGroup::inserter)) {
			auto* inserter_data = tile_layer->GetUniqueData<data::InserterData>();
			assert(inserter_data);

			const auto* proto_data = tile_layer->GetPrototypeData<data::Inserter>();
			assert(proto_data);

			InserterUpdate(logic_data, *proto_data, *inserter_data);
		}
	}
}
