// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 04/07/2020

#include "game/logic/item_logistics.h"

#include "data/prototype/entity/assembly_machine.h"
#include "data/prototype/entity/container_entity.h"
#include "data/prototype/entity/entity.h"
#include "data/prototype/entity/transport_line.h"
#include "data/prototype/item/item.h"
#include "game/logic/inserter_controller.h"
#include "game/logic/inventory_controller.h"

using namespace jactorio;

bool game::ItemDropOff::Initialize(const WorldData& world_data,
                                   data::UniqueDataBase& target_unique_data,
                                   const WorldData::WorldCoord world_x, const WorldData::WorldCoord world_y) {
	const data::Entity* entity =
		world_data.GetTile(world_x, world_y)
		          ->GetEntityPrototype(ChunkTile::ChunkLayer::entity);

	if (!entity)
		return false;

	switch (entity->Category()) {
	case data::DataCategory::container_entity:
		dropFunc_ = &ItemDropOff::InsertContainerEntity;
		break;

	case data::DataCategory::transport_belt:
		dropFunc_ = &ItemDropOff::InsertTransportBelt;
		break;

	case data::DataCategory::assembly_machine:
		dropFunc_ = &ItemDropOff::InsertAssemblyMachine;
		break;

	default:
		return false;
	}

	targetUniqueData_ = &target_unique_data;
	return true;
}

bool game::ItemDropOff::Initialize(const WorldData& world_data,
                                   data::UniqueDataBase& target_unique_data, const WorldData::WorldPair& world_coord) {
	return Initialize(world_data,
	                  target_unique_data,
	                  world_coord.first, world_coord.second);
}

bool game::ItemDropOff::InsertContainerEntity(const data::Item::Stack& item_stack, data::UniqueDataBase& unique_data,
                                              data::Orientation) const {
	auto& container_data = static_cast<data::ContainerEntityData&>(unique_data);
	if (!CanAddStack(container_data.inventory, item_stack).first)
		return false;

	AddStack(container_data.inventory, item_stack);
	return true;
}

bool game::ItemDropOff::InsertTransportBelt(const data::Item::Stack& item_stack, data::UniqueDataBase& unique_data,
                                            const data::Orientation orientation) const {
	assert(item_stack.count == 1);  // Can only insert 1 at a time

	auto& line_data = static_cast<data::TransportLineData&>(unique_data);

	bool use_line_left = false;
	// Decide whether to add item to left side or right side
	switch (line_data.lineSegment->direction) {
	case data::Orientation::up:
		switch (orientation) {
		case data::Orientation::up:
			break;
		case data::Orientation::right:
			use_line_left = true;
			break;
		case data::Orientation::down:
			return false;
		case data::Orientation::left:
			break;

		default:
			assert(false);
			break;
		}
		break;

	case data::Orientation::right:
		switch (orientation) {
		case data::Orientation::up:
			break;
		case data::Orientation::right:
			break;
		case data::Orientation::down:
			use_line_left = true;
			break;
		case data::Orientation::left:
			return false;

		default:
			assert(false);
			break;
		}
		break;

	case data::Orientation::down:
		switch (orientation) {
		case data::Orientation::up:
			return false;
		case data::Orientation::right:
			break;
		case data::Orientation::down:
			break;
		case data::Orientation::left:
			use_line_left = true;
			break;

		default:
			assert(false);
			break;
		}
		break;

	case data::Orientation::left:
		switch (orientation) {
		case data::Orientation::up:
			use_line_left = true;
			break;
		case data::Orientation::right:
			return false;
		case data::Orientation::down:
			break;
		case data::Orientation::left:
			break;

		default:
			assert(false);
			break;
		}
		break;

	default:
		assert(false);
		break;
	}

	constexpr double insertion_offset = 0.5;
	return line_data.lineSegment->TryInsertItem(use_line_left,
	                                            line_data.lineSegmentIndex + insertion_offset,
	                                            item_stack.item);
}

bool game::ItemDropOff::InsertAssemblyMachine(const data::Item::Stack& item_stack, data::UniqueDataBase& unique_data,
                                              data::Orientation) const {
	auto& machine_data = static_cast<data::AssemblyMachineData&>(unique_data);

	for (auto& slot : machine_data.ingredientInv) {
		if (slot.filter == item_stack.item) {
			slot.item = item_stack.item;
			slot.count += item_stack.count;
			return true;
		}
	}
	return false;
}

// ======================================================================

bool game::InserterPickup::Initialize(const WorldData& world_data,
                                      data::UniqueDataBase& target_unique_data,
                                      const WorldData::WorldCoord world_x, const WorldData::WorldCoord world_y) {
	const data::Entity* entity =
		world_data.GetTile(world_x, world_y)
		          ->GetEntityPrototype(ChunkTile::ChunkLayer::entity);

	if (!entity)
		return false;

	switch (entity->Category()) {
	case data::DataCategory::container_entity:
		pickupFunc_ = &InserterPickup::PickupContainerEntity;
		break;

	case data::DataCategory::transport_belt:
		pickupFunc_ = &InserterPickup::PickupTransportBelt;
		break;

	default:
		return false;
	}

	targetUniqueData_ = &target_unique_data;
	return true;
}

bool game::InserterPickup::Initialize(const WorldData& world_data,
                                      data::UniqueDataBase& target_unique_data,
                                      const WorldData::WorldPair& world_coord) {
	return Initialize(world_data, target_unique_data, world_coord.first, world_coord.second);
}

bool game::InserterPickup::PickupContainerEntity(data::ProtoUintT,
                                                 const data::RotationDegree& degree,
                                                 const data::Item::StackCount amount,
                                                 data::UniqueDataBase& unique_data,
                                                 data::Orientation,
                                                 data::Item::Stack& out_item_stack) const {
	if (degree != data::ToRotationDegree(kMaxInserterDegree))
		return false;

	auto& container = static_cast<data::ContainerEntityData&>(unique_data);


	const auto* target_item = GetFirstItem(container.inventory);

	out_item_stack = data::Item::Stack{target_item, amount};

	return RemoveInvItem(container.inventory, target_item, amount);
}

bool game::InserterPickup::PickupTransportBelt(const data::ProtoUintT inserter_tile_reach,
                                               const data::RotationDegree& degree,
                                               const data::Item::StackCount,
                                               data::UniqueDataBase& unique_data,
                                               const data::Orientation orientation,
                                               data::Item::Stack& out_item_stack) const {
	auto& line_data = static_cast<data::TransportLineData&>(unique_data);

	bool use_line_left = false;
	switch (line_data.lineSegment->direction) {
	case data::Orientation::up:
		switch (orientation) {
		case data::Orientation::down:
		case data::Orientation::left:
			use_line_left = true;
			break;

		default:
			break;
		}
		break;

	case data::Orientation::right:
		switch (orientation) {
		case data::Orientation::up:
		case data::Orientation::left:
			use_line_left = true;
			break;

		default:
			break;
		}
		break;

	case data::Orientation::down:
		switch (orientation) {
		case data::Orientation::up:
		case data::Orientation::right:
			use_line_left = true;
			break;

		default:
			break;
		}
		break;

	case data::Orientation::left:
		switch (orientation) {
		case data::Orientation::right:
		case data::Orientation::down:
			use_line_left = true;
			break;

		default:
			break;
		}
		break;

	default:
		assert(false);
		break;
	}

	const auto pickup_offset =
		line_data.lineSegmentIndex +
		GetInserterArmOffset(degree.getAsInteger(), inserter_tile_reach);

	const auto* item = line_data.lineSegment->TryPopItemAbs(use_line_left, pickup_offset);

	if (item != nullptr) {
		line_data.lineSegment->GetSide(use_line_left).index = 0;

		out_item_stack = data::Item::Stack{item, 1};

		return true;
	}
	return false;
}
