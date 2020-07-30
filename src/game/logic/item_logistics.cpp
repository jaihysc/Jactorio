// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

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
                                   const WorldCoordAxis world_x, const WorldCoordAxis world_y) {
	const data::Entity* entity =
		world_data.GetTile(world_x, world_y)
		          ->GetEntityPrototype();

	if (!entity)
		return false;

	switch (entity->Category()) {
	case data::DataCategory::container_entity:
		dropFunc_ = &ItemDropOff::InsertContainerEntity;
		canDropFunc_ = &ItemDropOff::CanInsertContainerEntity;
		break;

	case data::DataCategory::transport_belt:
		dropFunc_ = &ItemDropOff::InsertTransportBelt;
		canDropFunc_ = &ItemDropOff::CanInsertTransportBelt;
		break;

	case data::DataCategory::assembly_machine:
		dropFunc_ = &ItemDropOff::InsertAssemblyMachine;
		canDropFunc_ = &ItemDropOff::CanInsertAssemblyMachine;
		break;

	default:
		return false;
	}

	targetProtoData_  = entity;
	targetUniqueData_ = &target_unique_data;

	return true;
}

bool game::ItemDropOff::Initialize(const WorldData& world_data,
                                   data::UniqueDataBase& target_unique_data, const WorldCoord& world_coord) {
	return Initialize(world_data,
	                  target_unique_data,
	                  world_coord.x, world_coord.y);
}

bool game::ItemDropOff::CanInsertContainerEntity(const DropOffParams&) const {
	return true;
}

bool game::ItemDropOff::InsertContainerEntity(const DropOffParams& args) const {
	auto& container_data = static_cast<data::ContainerEntityData&>(args.uniqueData);
	if (!CanAddStack(container_data.inventory, args.itemStack).first)
		return false;

	AddStack(container_data.inventory, args.itemStack);
	return true;
}

bool game::ItemDropOff::CanInsertTransportBelt(const DropOffParams&) const {
	return true;
}

bool game::ItemDropOff::InsertTransportBelt(const DropOffParams& args) const {
	assert(args.itemStack.count == 1);  // Can only insert 1 at a time

	auto& line_data = static_cast<data::TransportLineData&>(args.uniqueData);

	bool use_line_left = false;
	// Decide whether to add item to left side or right side
	switch (line_data.lineSegment->direction) {
	case data::Orientation::up:
		switch (args.orientation) {
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
		switch (args.orientation) {
		case data::Orientation::up:
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
		switch (args.orientation) {
		case data::Orientation::up:
			return false;
		case data::Orientation::right:
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
		switch (args.orientation) {
		case data::Orientation::up:
			use_line_left = true;
			break;
		case data::Orientation::right:
			return false;
		case data::Orientation::down:
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
	                                            args.itemStack.item);
}

bool game::ItemDropOff::CanInsertAssemblyMachine(const DropOffParams& args) const {
	auto& machine_data = static_cast<data::AssemblyMachineData&>(args.uniqueData);

	// No recipe
	if (machine_data.GetRecipe() == nullptr)
		return false;

	for (auto& slot : machine_data.ingredientInv) {
		if (slot.filter == args.itemStack.item)
			return true;
	}

	return false;
}

bool game::ItemDropOff::InsertAssemblyMachine(const DropOffParams& args) const {
	assert(args.itemStack.item);
	assert(args.itemStack.count > 0);

	auto& machine_data = static_cast<data::AssemblyMachineData&>(args.uniqueData);

	for (auto& slot : machine_data.ingredientInv) {
		if (slot.filter == args.itemStack.item) {
			if (slot.count + args.itemStack.count > args.itemStack.item->stackSize)
				continue;

			slot.item = args.itemStack.item;
			slot.count += args.itemStack.count;

			assert(targetProtoData_);
			static_cast<const data::AssemblyMachine*>(targetProtoData_)
				->TryBeginCrafting(args.logicData, machine_data);
			return true;
		}
	}
	return false;
}

// ======================================================================

bool game::InserterPickup::Initialize(const WorldData& world_data,
                                      data::UniqueDataBase& target_unique_data,
                                      const WorldCoordAxis world_x, const WorldCoordAxis world_y) {
	const data::Entity* entity =
		world_data.GetTile(world_x, world_y)
		          ->GetEntityPrototype();

	if (!entity)
		return false;

	switch (entity->Category()) {
	case data::DataCategory::container_entity:
		pickupFunc_ = &InserterPickup::PickupContainerEntity;
		getPickupFunc_ = &InserterPickup::GetPickupContainerEntity;
		break;

	case data::DataCategory::transport_belt:
		pickupFunc_ = &InserterPickup::PickupTransportBelt;
		getPickupFunc_ = &InserterPickup::GetPickupTransportBelt;
		break;

	case data::DataCategory::assembly_machine:
		pickupFunc_ = &InserterPickup::PickupAssemblyMachine;
		getPickupFunc_ = &InserterPickup::GetPickupAssemblyMachine;
		break;

	default:
		return false;
	}

	targetProtoData_  = entity;
	targetUniqueData_ = &target_unique_data;

	return true;
}

bool game::InserterPickup::Initialize(const WorldData& world_data,
                                      data::UniqueDataBase& target_unique_data,
                                      const WorldCoord& world_coord) {
	return Initialize(world_data, target_unique_data, world_coord.x, world_coord.y);
}


game::InserterPickup::GetPickupReturn game::InserterPickup::GetPickupContainerEntity(const PickupParams& args) const {
	auto& container = static_cast<data::ContainerEntityData&>(args.uniqueData);
	return GetFirstItem(container.inventory);
}

game::InserterPickup::PickupReturn game::InserterPickup::PickupContainerEntity(const PickupParams& args) const {
	if (!IsAtMaxDegree(args.degree))
		return {false, {}};

	auto& container = static_cast<data::ContainerEntityData&>(args.uniqueData);


	const auto* target_item = GetFirstItem(container.inventory);

	return {RemoveInvItem(container.inventory, target_item, args.amount), {target_item, args.amount}};
}


game::InserterPickup::GetPickupReturn game::InserterPickup::GetPickupTransportBelt(const PickupParams& args) const {
	auto& line_data = static_cast<data::TransportLineData&>(args.uniqueData);

	const auto props         = GetBeltPickupProps(args);
	const bool use_line_left = props.first;
	const auto pickup_offset = props.second;

	return line_data.lineSegment->GetItemAbs(use_line_left, pickup_offset).second.second;
}


game::InserterPickup::PickupReturn game::InserterPickup::PickupTransportBelt(const PickupParams& args) const {
	auto& line_data = static_cast<data::TransportLineData&>(args.uniqueData);

	const auto props         = GetBeltPickupProps(args);
	const bool use_line_left = props.first;
	const auto pickup_offset = props.second;

	const auto* item = line_data.lineSegment->TryPopItemAbs(use_line_left, pickup_offset);

	if (item != nullptr) {
		line_data.lineSegment->GetSide(use_line_left).index = 0;

		return {true, {item, 1}};
	}
	return {false, {}};
}


game::InserterPickup::GetPickupReturn game::InserterPickup::GetPickupAssemblyMachine(const PickupParams& args) const {
	assert(args.amount > 0);

	auto& machine_data = static_cast<data::AssemblyMachineData&>(args.uniqueData);

	if (!machine_data.HasRecipe())
		return nullptr;

	auto& product_stack = machine_data.productInv[0];
	return product_stack.filter;
}

game::InserterPickup::PickupReturn game::InserterPickup::PickupAssemblyMachine(const PickupParams& args) const {
	assert(args.amount > 0);

	if (!IsAtMaxDegree(args.degree))
		return {false, {}};


	auto& machine_data = static_cast<data::AssemblyMachineData&>(args.uniqueData);

	if (!machine_data.HasRecipe())
		return {false, {}};

	auto& product_stack = machine_data.productInv[0];

	// Not enough to pick up
	if (product_stack.count < args.amount)
		return {false, {}};

	product_stack.count -= args.amount;

	const auto* asm_machine = static_cast<const data::AssemblyMachine*>(targetProtoData_);
	assert(asm_machine);
	asm_machine->TryBeginCrafting(args.logicData, machine_data);

	return {true, {product_stack.item, args.amount}};
}


bool game::InserterPickup::IsAtMaxDegree(const data::RotationDegree& degree) {
	return degree == data::ToRotationDegree(kMaxInserterDegree);
}

std::pair<bool, double> game::InserterPickup::GetBeltPickupProps(const PickupParams& args) {
	auto& line_data = static_cast<data::TransportLineData&>(args.uniqueData);

	bool use_line_left = false;
	switch (line_data.lineSegment->direction) {
	case data::Orientation::up:
		switch (args.orientation) {
		case data::Orientation::down:
		case data::Orientation::left:
			use_line_left = true;
			break;

		default:
			break;
		}
		break;

	case data::Orientation::right:
		switch (args.orientation) {
		case data::Orientation::up:
		case data::Orientation::left:
			use_line_left = true;
			break;

		default:
			break;
		}
		break;

	case data::Orientation::down:
		switch (args.orientation) {
		case data::Orientation::up:
		case data::Orientation::right:
			use_line_left = true;
			break;

		default:
			break;
		}
		break;

	case data::Orientation::left:
		switch (args.orientation) {
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

	auto pickup_offset = line_data.lineSegmentIndex +
		GetInserterArmOffset(args.degree.getAsInteger(), args.inserterTileReach);

	return {use_line_left, pickup_offset};
}
