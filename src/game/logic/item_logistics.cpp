// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/logic/item_logistics.h"

#include "data/prototype/abstract_proto/transport_line.h"
#include "data/prototype/assembly_machine.h"
#include "data/prototype/container_entity.h"
#include "data/prototype/item.h"
#include "game/logic/inserter_controller.h"
#include "game/logic/inventory_controller.h"
#include "game/logic/transport_line_controller.h"

using namespace jactorio;

bool game::ItemDropOff::Initialize(WorldData& world_data, const WorldCoordAxis world_x, const WorldCoordAxis world_y) {
    auto* tile = world_data.GetTile(world_x, world_y);
    assert(tile != nullptr);

    auto& layer = tile->GetLayer(TileLayer::entity);

    if (layer.prototypeData == nullptr)
        return false;

    switch (layer.prototypeData->Category()) {
    case data::DataCategory::container_entity:
        dropFunc_    = &ItemDropOff::InsertContainerEntity;
        canDropFunc_ = &ItemDropOff::CanInsertContainerEntity;
        break;

    case data::DataCategory::transport_belt:
        dropFunc_    = &ItemDropOff::InsertTransportBelt;
        canDropFunc_ = &ItemDropOff::CanInsertTransportBelt;
        break;

    case data::DataCategory::assembly_machine:
        dropFunc_    = &ItemDropOff::InsertAssemblyMachine;
        canDropFunc_ = &ItemDropOff::CanInsertAssemblyMachine;
        break;

    default:
        return false;
    }

    assert(layer.GetUniqueData() != nullptr);

    targetProtoData_  = layer.prototypeData.Get();
    targetUniqueData_ = layer.GetUniqueData();

    return true;
}

bool game::ItemDropOff::CanInsertContainerEntity(const DropOffParams& /*params*/) const {
    return true;
}

bool game::ItemDropOff::InsertContainerEntity(const DropOffParams& params) const {
    auto& container_data = static_cast<data::ContainerEntityData&>(params.uniqueData);
    if (!CanAddStack(container_data.inventory, params.itemStack).first)
        return false;

    AddStack(container_data.inventory, params.itemStack);
    return true;
}

void GetAdjustedLineOffset(const bool use_line_left,
                           data::LineDistT& pickup_offset,
                           const data::TransportLineData& line_data) {
    game::TransportSegment::ApplyTerminationDeduction(use_line_left,
                                                      line_data.lineSegment->terminationType,
                                                      game::TransportSegment::TerminationType::straight,
                                                      pickup_offset);
}

bool game::ItemDropOff::CanInsertTransportBelt(const DropOffParams& /*params*/) const {
    return true;
}

bool game::ItemDropOff::InsertTransportBelt(const DropOffParams& params) const {
    assert(params.itemStack.count == 1); // Can only insert 1 at a time

    auto& line_data = static_cast<data::TransportLineData&>(params.uniqueData);

    bool use_line_left = false;
    // Decide whether to add item to left side or right side
    switch (line_data.lineSegment->direction) {
    case data::Orientation::up:
        switch (params.orientation) {
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
        switch (params.orientation) {
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
        switch (params.orientation) {
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
        switch (params.orientation) {
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

    constexpr double insertion_offset_base = 0.5;
    auto offset                            = data::LineDistT(line_data.lineSegmentIndex + insertion_offset_base);

    GetAdjustedLineOffset(use_line_left, offset, line_data);
    return line_data.lineSegment->TryInsertItem(use_line_left, offset.getAsDouble(), *params.itemStack.item);
}

bool game::ItemDropOff::CanInsertAssemblyMachine(const DropOffParams& params) const {
    auto& machine_data = static_cast<data::AssemblyMachineData&>(params.uniqueData);

    constexpr int max_ingredient_sets = 2; // Will allow filling to (ingredient count for crafting) * 2
    static_assert(max_ingredient_sets >= 1);

    // No recipe
    const auto* recipe = machine_data.GetRecipe();
    if (recipe == nullptr)
        return false;

    for (size_t i = 0; i < machine_data.ingredientInv.size(); ++i) {
        auto& slot = machine_data.ingredientInv[i];

        if (slot.filter == params.itemStack.item) {

            // Must be less than: max sets(multiples) of ingredient OR maximum stack size
            const auto max_count = max_ingredient_sets * recipe->ingredients[i].second;
            return !(slot.count >= max_count || slot.count >= slot.filter->stackSize);
        }
    }

    return false;
}

bool game::ItemDropOff::InsertAssemblyMachine(const DropOffParams& params) const {
    assert(params.itemStack.item != nullptr);
    assert(params.itemStack.count > 0);

    auto& machine_data = static_cast<data::AssemblyMachineData&>(params.uniqueData);

    for (auto& slot : machine_data.ingredientInv) {
        if (slot.filter == params.itemStack.item) {
            if (slot.count + params.itemStack.count > params.itemStack.item->stackSize)
                continue;

            slot.item = params.itemStack.item;
            slot.count += params.itemStack.count;

            assert(targetProtoData_);
            static_cast<const data::AssemblyMachine*>(targetProtoData_)
                ->TryBeginCrafting(params.logicData, machine_data);
            return true;
        }
    }
    return false;
}

// ======================================================================

bool game::InserterPickup::Initialize(WorldData& world_data,
                                      const WorldCoordAxis world_x,
                                      const WorldCoordAxis world_y) {
    auto* tile = world_data.GetTile(world_x, world_y);
    assert(tile != nullptr);

    auto& layer = tile->GetLayer(TileLayer::entity);

    if (layer.prototypeData == nullptr)
        return false;

    switch (layer.prototypeData->Category()) {
    case data::DataCategory::container_entity:
        pickupFunc_    = &InserterPickup::PickupContainerEntity;
        getPickupFunc_ = &InserterPickup::GetPickupContainerEntity;
        break;

    case data::DataCategory::transport_belt:
        pickupFunc_    = &InserterPickup::PickupTransportBelt;
        getPickupFunc_ = &InserterPickup::GetPickupTransportBelt;
        break;

    case data::DataCategory::assembly_machine:
        pickupFunc_    = &InserterPickup::PickupAssemblyMachine;
        getPickupFunc_ = &InserterPickup::GetPickupAssemblyMachine;
        break;

    default:
        return false;
    }

    assert(layer.GetUniqueData() != nullptr);

    targetProtoData_  = layer.prototypeData.Get();
    targetUniqueData_ = layer.GetUniqueData();

    return true;
}

game::InserterPickup::GetPickupReturn game::InserterPickup::GetPickupContainerEntity(const PickupParams& params) const {
    auto& container = static_cast<data::ContainerEntityData&>(params.uniqueData);
    return GetFirstItem(container.inventory);
}

game::InserterPickup::PickupReturn game::InserterPickup::PickupContainerEntity(const PickupParams& params) const {
    if (!IsAtMaxDegree(params.degree))
        return {false, {}};

    auto& container = static_cast<data::ContainerEntityData&>(params.uniqueData);


    const auto* target_item = GetFirstItem(container.inventory);

    return {RemoveInvItem(container.inventory, target_item, params.amount), {target_item, params.amount}};
}


game::InserterPickup::GetPickupReturn game::InserterPickup::GetPickupTransportBelt(const PickupParams& params) const {
    auto& line_data = static_cast<data::TransportLineData&>(params.uniqueData);

    const auto props         = GetBeltPickupProps(params);
    const bool use_line_left = props.first;
    const auto pickup_offset = props.second; // Cannot capture structured binding

    auto get_item = [&](const bool left_lane) {
        auto adjusted_pickup_offset = pickup_offset;

        GetAdjustedLineOffset(left_lane, adjusted_pickup_offset, line_data);

        auto [dq_index, line_item] = line_data.lineSegment->GetItem(left_lane, adjusted_pickup_offset.getAsDouble());
        return line_item.item.Get();
    };


    auto* item = get_item(use_line_left);
    if (item == nullptr) { // Try picking up from other lane if preferred lane fails
        item = get_item(!use_line_left);
    }

    return item;
}


game::InserterPickup::PickupReturn game::InserterPickup::PickupTransportBelt(const PickupParams& params) const {
    auto& line_data = static_cast<data::TransportLineData&>(params.uniqueData);

    const auto props          = GetBeltPickupProps(params);
    bool use_line_left        = props.first;
    const auto& pickup_offset = props.second;

    auto try_pickup_item = [&](const bool left_lane) {
        auto adjusted_pickup_offset = pickup_offset;

        GetAdjustedLineOffset(left_lane, adjusted_pickup_offset, line_data);
        return line_data.lineSegment->TryPopItem(left_lane, adjusted_pickup_offset.getAsDouble());
    };


    auto* item = try_pickup_item(use_line_left);
    if (item == nullptr) {
        // Try picking up from other lane if preferred lane fails
        // use_line_left itself must be inverted so the handling after an item was picked up utilizes the correct lane
        use_line_left = !use_line_left;

        item = try_pickup_item(use_line_left);
    }

    if (item != nullptr) {
        line_data.lineSegment->GetSide(use_line_left).index = 0;

        return {true, {item, 1}};
    }
    return {false, {}};
}


game::InserterPickup::GetPickupReturn game::InserterPickup::GetPickupAssemblyMachine(const PickupParams& params) const {
    assert(params.amount > 0);

    auto& machine_data = static_cast<data::AssemblyMachineData&>(params.uniqueData);

    if (!machine_data.HasRecipe())
        return nullptr;

    auto& product_stack = machine_data.productInv[0];
    return product_stack.filter.Get();
}

game::InserterPickup::PickupReturn game::InserterPickup::PickupAssemblyMachine(const PickupParams& params) const {
    assert(params.amount > 0);

    if (!IsAtMaxDegree(params.degree))
        return {false, {}};


    auto& machine_data = static_cast<data::AssemblyMachineData&>(params.uniqueData);

    if (!machine_data.HasRecipe())
        return {false, {}};

    auto& product_stack = machine_data.productInv[0];

    // Not enough to pick up
    if (product_stack.count < params.amount)
        return {false, {}};

    product_stack.count -= params.amount;

    const auto* asm_machine = static_cast<const data::AssemblyMachine*>(targetProtoData_);
    assert(asm_machine);
    asm_machine->TryBeginCrafting(params.logicData, machine_data);

    return {true, {product_stack.item, params.amount}};
}


bool game::InserterPickup::IsAtMaxDegree(const data::RotationDegreeT& degree) {
    return degree == data::RotationDegreeT(kMaxInserterDegree);
}

std::pair<bool, data::LineDistT> game::InserterPickup::GetBeltPickupProps(const PickupParams& params) {
    auto& line_data = static_cast<data::TransportLineData&>(params.uniqueData);

    bool use_line_left = false;
    switch (line_data.lineSegment->direction) {
    case data::Orientation::up:
        switch (params.orientation) {
        case data::Orientation::down:
        case data::Orientation::left:
            use_line_left = true;
            break;

        default:
            break;
        }
        break;

    case data::Orientation::right:
        switch (params.orientation) {
        case data::Orientation::up:
        case data::Orientation::left:
            use_line_left = true;
            break;

        default:
            break;
        }
        break;

    case data::Orientation::down:
        switch (params.orientation) {
        case data::Orientation::up:
        case data::Orientation::right:
            use_line_left = true;
            break;

        default:
            break;
        }
        break;

    case data::Orientation::left:
        switch (params.orientation) {
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

    auto pickup_offset = data::LineDistT(
        line_data.lineSegmentIndex +
        GetInserterArmOffset(core::SafeCast<core::TIntDegree>(params.degree.getAsInteger()), params.inserterTileReach));

    return {use_line_left, pickup_offset};
}
