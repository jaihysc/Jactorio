// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 04/07/2020

#include "game/logic/item_logistics.h"

#include "data/prototype/entity/container_entity.h"
#include "data/prototype/entity/entity.h"
#include "data/prototype/entity/transport/transport_line.h"
#include "data/prototype/item/item.h"
#include "game/logic/inserter_controller.h"
#include "game/logic/inventory_controller.h"

bool jactorio::game::ItemDropOff::Initialize(const WorldData& world_data,
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

	default:
		return false;
	}

	targetUniqueData_ = &target_unique_data;
	return true;
}

bool jactorio::game::ItemDropOff::Initialize(const WorldData& world_data,
                                             data::UniqueDataBase& target_unique_data, const WorldData::WorldPair& world_coord) {
	return Initialize(world_data,
	                  target_unique_data,
	                  world_coord.first, world_coord.second);
}

bool jactorio::game::ItemDropOff::InsertContainerEntity(const data::ItemStack& item_stack, data::UniqueDataBase& unique_data,
                                                        data::Orientation) const {
	auto& container_data = static_cast<data::ContainerEntityData&>(unique_data);
	if (!CanAddStack(container_data.inventory, container_data.size, item_stack))
		return false;

	AddStack(container_data.inventory, container_data.size, item_stack);
	return true;
}

bool jactorio::game::ItemDropOff::InsertTransportBelt(const data::ItemStack& item_stack, data::UniqueDataBase& unique_data,
                                                      const data::Orientation orientation) const {
	assert(item_stack.second == 1);  // Can only insert 1 at a time

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
	                                            item_stack.first);
}

// ======================================================================

bool jactorio::game::InserterPickup::Initialize(const WorldData& world_data,
                                                data::UniqueDataBase& target_unique_data,
                                                WorldData::WorldCoord world_x, WorldData::WorldCoord world_y) {
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

bool jactorio::game::InserterPickup::Initialize(const WorldData& world_data,
                                                data::UniqueDataBase& target_unique_data,
                                                const WorldData::WorldPair& world_coord) {
	return Initialize(world_data, target_unique_data, world_coord.first, world_coord.second);
}

bool jactorio::game::InserterPickup::PickupContainerEntity(const data::RotationDegree& degree,
                                                           const data::ItemStack::second_type amount,
                                                           data::UniqueDataBase& unique_data,
                                                           data::Orientation,
                                                           data::ItemStack& out_item_stack) const {
	if (degree != data::ToRotationDegree(kMaxInserterDegree))
		return false;

	auto& container = static_cast<data::ContainerEntityData&>(unique_data);


	const auto* target_item = GetFirstItem(container.inventory, container.size);

	out_item_stack = data::ItemStack{target_item, amount};

	return RemoveInvItem(container.inventory, container.size,
	                     target_item,
	                     amount);
}

bool jactorio::game::InserterPickup::PickupTransportBelt(const data::RotationDegree& degree,
                                                         const data::ItemStack::second_type amount,
                                                         data::UniqueDataBase& unique_data,
                                                         const data::Orientation orientation,
                                                         data::ItemStack& out_item_stack) const {
	if (amount != 1) // TODO
		LOG_MESSAGE_f(warning, "Inserters will only pick up 1 item at the moment, provided amount: %d", amount);

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
		GetInserterArmOffset(degree.getAsInteger(), 1);  // TODO different target distances

	const auto* item = line_data.lineSegment->TryPopItemAbs(use_line_left, pickup_offset);

	out_item_stack = data::ItemStack{item, amount};
	return item != nullptr;
}
