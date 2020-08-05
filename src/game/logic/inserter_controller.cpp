// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/logic/inserter_controller.h"

#include <vector>

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


// ======================================================================


// Inserters are updated in stages together so that logic order does not affect inserter behavior
// E.g Inserter1 drop off             , inserter2 picks up | 1 logic update
//     Inserter2 has nothing to pickup, inserter1 drop off | 1 logic update

struct InserterUpdateProps
{
	const data::Inserter& proto;
	data::InserterData& data;
};

using DropoffQueue = std::vector<InserterUpdateProps>;
using PickupQueue = std::vector<InserterUpdateProps>;

///
/// \brief Rotates inserters, queues inserters awaiting dropoff and pickup handling
void RotateInserters(DropoffQueue& dropoff_queue, PickupQueue& pickup_queue,
                     const InserterUpdateProps& props) {
	using namespace game;

	assert(props.proto.rotationSpeed.getAsDouble() != 0);

	switch (props.data.status) {

	case data::InserterData::Status::dropoff:
		props.data.rotationDegree -= props.proto.rotationSpeed;

		if (props.data.rotationDegree <= data::ToRotationDegree(kMinInserterDegree)) {
			props.data.rotationDegree = 0;  // Prevents underflow if the inserter sits idle for a long time
			dropoff_queue.push_back(props);
		}

		return;

	case data::InserterData::Status::pickup:
		// Rotate the inserter
		props.data.rotationDegree += props.proto.rotationSpeed;

		if (props.data.rotationDegree > data::ToRotationDegree(kMaxInserterDegree)) {
			props.data.rotationDegree = kMaxInserterDegree;  // Prevents overflow
			pickup_queue.push_back(props);
		}

		return;

	default:
		assert(false);
	}

}

void ProcessInserterDropoff(const DropoffQueue& dropoff_queue, game::LogicData& logic_data) {
	for (const auto& inserter_prop : dropoff_queue) {
		auto& inserter_data = inserter_prop.data;

		if (inserter_data.dropoff.DropOff(logic_data, inserter_data.heldItem)) {
			inserter_data.status = data::InserterData::Status::pickup;
		}
	}

}

void ProcessInserterPickup(const PickupQueue& pickup_queue, game::LogicData& logic_data) {
	for (const auto& inserter_prop : pickup_queue) {
		auto& inserter_data        = inserter_prop.data;
		const auto& inserter_proto = inserter_prop.proto;

		constexpr int pickup_amount = 1;

		const auto* to_be_picked_item =
			inserter_data.pickup.GetPickup(logic_data,
			                               inserter_proto.tileReach,
			                               inserter_data.rotationDegree);

		// Do not pick up item if it cannot be dropped off
		if (!inserter_data.dropoff.CanDropOff(logic_data, to_be_picked_item))
			continue;


		const auto result = inserter_data.pickup.Pickup(logic_data,
		                                                inserter_proto.tileReach,
		                                                inserter_data.rotationDegree,
		                                                pickup_amount);
		if (result.first) {
			inserter_data.heldItem = result.second;

			inserter_data.status = data::InserterData::Status::dropoff;
		}
	}

}

void game::InserterLogicUpdate(WorldData& world_data, LogicData& logic_data) {
	DropoffQueue dropoff_queue{};
	PickupQueue pickup_queue{};

	for (auto* chunk : world_data.LogicGetChunks()) {
		for (auto* tile_layer : chunk->GetLogicGroup(Chunk::LogicGroup::inserter)) {
			auto* inserter_data = tile_layer->GetUniqueData<data::InserterData>();
			assert(inserter_data);

			const auto* proto_data = tile_layer->GetPrototypeData<data::Inserter>();
			assert(proto_data);

			RotateInserters(dropoff_queue, pickup_queue, {*proto_data, *inserter_data});
		}
	}

	ProcessInserterDropoff(dropoff_queue, logic_data);
	ProcessInserterPickup(pickup_queue, logic_data);
}
