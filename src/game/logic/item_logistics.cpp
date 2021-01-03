// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/logic/item_logistics.h"

#include "proto/abstract/conveyor.h"
#include "proto/assembly_machine.h"
#include "proto/container_entity.h"
#include "proto/item.h"
#include "proto/recipe.h"

#include "game/logic/inserter_controller.h"
#include "game/logic/inventory_controller.h"
#include "game/world/world_data.h"

using namespace jactorio;

bool game::ItemDropOff::Initialize(WorldData& world_data, const WorldCoordAxis world_x, const WorldCoordAxis world_y) {
    auto* tile = world_data.GetTile(world_x, world_y);
    assert(tile != nullptr);

    auto& layer = tile->GetLayer(TileLayer::entity);

    if (layer.GetPrototype() == nullptr)
        return false;

    switch (layer.GetPrototype()->GetCategory()) {
    case proto::Category::container_entity:
        dropFunc_    = &ItemDropOff::InsertContainerEntity;
        canDropFunc_ = &ItemDropOff::CanInsertContainerEntity;
        break;

    case proto::Category::transport_belt:
        dropFunc_    = &ItemDropOff::InsertTransportBelt;
        canDropFunc_ = &ItemDropOff::CanInsertTransportBelt;
        break;

    case proto::Category::assembly_machine:
        dropFunc_    = &ItemDropOff::InsertAssemblyMachine;
        canDropFunc_ = &ItemDropOff::CanInsertAssemblyMachine;
        break;

    default:
        return false;
    }

    assert(layer.GetUniqueData() != nullptr);

    targetProtoData_  = layer.GetPrototype();
    targetUniqueData_ = layer.GetUniqueData();

    return true;
}

bool game::ItemDropOff::CanInsertContainerEntity(const DropOffParams& /*params*/) const {
    return true;
}

bool game::ItemDropOff::InsertContainerEntity(const DropOffParams& params) const {
    auto& container_data = static_cast<proto::ContainerEntityData&>(params.uniqueData);
    if (!CanAddStack(container_data.inventory, params.itemStack).first)
        return false;

    AddStack(container_data.inventory, params.itemStack);
    return true;
}

void GetAdjustedLineOffset(const bool use_line_left,
                           proto::LineDistT& pickup_offset,
                           const proto::ConveyorData& line_data) {
    game::ConveyorStruct::ApplyTerminationDeduction(use_line_left,
                                                    line_data.structure->terminationType,
                                                    game::ConveyorStruct::TerminationType::straight,
                                                    pickup_offset);
}

bool game::ItemDropOff::CanInsertTransportBelt(const DropOffParams& /*params*/) const {
    return true;
}

bool game::ItemDropOff::InsertTransportBelt(const DropOffParams& params) const {
    assert(params.itemStack.count == 1); // Can only insert 1 at a time

    auto& line_data = static_cast<proto::ConveyorData&>(params.uniqueData);

    bool use_line_left = false;
    // Decide whether to add item to left side or right side
    switch (line_data.structure->direction) {
    case Orientation::up:
        switch (params.orientation) {
        case Orientation::up:
            break;
        case Orientation::right:
            use_line_left = true;
            break;
        case Orientation::down:
            return false;
        case Orientation::left:
            break;

        default:
            assert(false);
            break;
        }
        break;

    case Orientation::right:
        switch (params.orientation) {
        case Orientation::up:
        case Orientation::right:
            break;
        case Orientation::down:
            use_line_left = true;
            break;
        case Orientation::left:
            return false;

        default:
            assert(false);
            break;
        }
        break;

    case Orientation::down:
        switch (params.orientation) {
        case Orientation::up:
            return false;
        case Orientation::right:
        case Orientation::down:
            break;
        case Orientation::left:
            use_line_left = true;
            break;

        default:
            assert(false);
            break;
        }
        break;

    case Orientation::left:
        switch (params.orientation) {
        case Orientation::up:
            use_line_left = true;
            break;
        case Orientation::right:
            return false;
        case Orientation::down:
        case Orientation::left:
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
    auto offset                            = proto::LineDistT(line_data.structIndex + insertion_offset_base);

    GetAdjustedLineOffset(use_line_left, offset, line_data);
    return line_data.structure->TryInsertItem(use_line_left, offset.getAsDouble(), *params.itemStack.item);
}

bool game::ItemDropOff::CanInsertAssemblyMachine(const DropOffParams& params) const {
    auto& machine_data = static_cast<proto::AssemblyMachineData&>(params.uniqueData);

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

    auto& machine_data = static_cast<proto::AssemblyMachineData&>(params.uniqueData);

    for (auto& slot : machine_data.ingredientInv) {
        if (slot.filter == params.itemStack.item) {
            if (slot.count + params.itemStack.count > params.itemStack.item->stackSize)
                continue;

            slot.item = params.itemStack.item;
            slot.count += params.itemStack.count;

            assert(targetProtoData_);
            static_cast<const proto::AssemblyMachine*>(targetProtoData_)
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

    if (layer.GetPrototype() == nullptr)
        return false;

    switch (layer.GetPrototype()->GetCategory()) {
    case proto::Category::container_entity:
        pickupFunc_    = &InserterPickup::PickupContainerEntity;
        getPickupFunc_ = &InserterPickup::GetPickupContainerEntity;
        break;

    case proto::Category::transport_belt:
        pickupFunc_    = &InserterPickup::PickupTransportBelt;
        getPickupFunc_ = &InserterPickup::GetPickupTransportBelt;
        break;

    case proto::Category::assembly_machine:
        pickupFunc_    = &InserterPickup::PickupAssemblyMachine;
        getPickupFunc_ = &InserterPickup::GetPickupAssemblyMachine;
        break;

    default:
        return false;
    }

    assert(layer.GetUniqueData() != nullptr);

    targetProtoData_  = layer.GetPrototype();
    targetUniqueData_ = layer.GetUniqueData();

    return true;
}

game::InserterPickup::GetPickupReturn game::InserterPickup::GetPickupContainerEntity(const PickupParams& params) const {
    auto& container = static_cast<proto::ContainerEntityData&>(params.uniqueData);
    return GetFirstItem(container.inventory);
}

game::InserterPickup::PickupReturn game::InserterPickup::PickupContainerEntity(const PickupParams& params) const {
    if (!IsAtMaxDegree(params.degree))
        return {false, {}};

    auto& container = static_cast<proto::ContainerEntityData&>(params.uniqueData);


    const auto* target_item = GetFirstItem(container.inventory);

    return {RemoveInvItem(container.inventory, target_item, params.amount), {target_item, params.amount}};
}


game::InserterPickup::GetPickupReturn game::InserterPickup::GetPickupTransportBelt(const PickupParams& params) const {
    auto& line_data = static_cast<proto::ConveyorData&>(params.uniqueData);

    const auto props         = GetBeltPickupProps(params);
    const bool use_line_left = props.first;
    const auto pickup_offset = props.second; // Cannot capture structured binding

    auto get_item = [&](const bool left_lane) {
        auto adjusted_pickup_offset = pickup_offset;

        GetAdjustedLineOffset(left_lane, adjusted_pickup_offset, line_data);

        auto [dq_index, line_item] = line_data.structure->GetItem(left_lane, adjusted_pickup_offset.getAsDouble());
        return line_item.item.Get();
    };


    auto* item = get_item(use_line_left);
    if (item == nullptr) { // Try picking up from other lane if preferred lane fails
        item = get_item(!use_line_left);
    }

    return item;
}


game::InserterPickup::PickupReturn game::InserterPickup::PickupTransportBelt(const PickupParams& params) const {
    auto& line_data = static_cast<proto::ConveyorData&>(params.uniqueData);

    const auto props          = GetBeltPickupProps(params);
    bool use_line_left        = props.first;
    const auto& pickup_offset = props.second;

    auto try_pickup_item = [&](const bool left_lane) {
        auto adjusted_pickup_offset = pickup_offset;

        GetAdjustedLineOffset(left_lane, adjusted_pickup_offset, line_data);
        return line_data.structure->TryPopItem(left_lane, adjusted_pickup_offset.getAsDouble());
    };


    auto* item = try_pickup_item(use_line_left);
    if (item == nullptr) {
        // Try picking up from other lane if preferred lane fails
        // use_line_left itself must be inverted so the handling after an item was picked up utilizes the correct lane
        use_line_left = !use_line_left;

        item = try_pickup_item(use_line_left);
    }

    if (item != nullptr) {
        line_data.structure->GetSide(use_line_left).index = 0;

        return {true, {item, 1}};
    }
    return {false, {}};
}


game::InserterPickup::GetPickupReturn game::InserterPickup::GetPickupAssemblyMachine(const PickupParams& params) const {
    assert(params.amount > 0);

    auto& machine_data = static_cast<proto::AssemblyMachineData&>(params.uniqueData);

    if (!machine_data.HasRecipe())
        return nullptr;

    auto& product_stack = machine_data.productInv[0];
    return product_stack.filter.Get();
}

game::InserterPickup::PickupReturn game::InserterPickup::PickupAssemblyMachine(const PickupParams& params) const {
    assert(params.amount > 0);

    if (!IsAtMaxDegree(params.degree))
        return {false, {}};


    auto& machine_data = static_cast<proto::AssemblyMachineData&>(params.uniqueData);

    if (!machine_data.HasRecipe())
        return {false, {}};

    auto& product_stack = machine_data.productInv[0];

    // Not enough to pick up
    if (product_stack.count < params.amount)
        return {false, {}};

    product_stack.count -= params.amount;

    const auto* asm_machine = static_cast<const proto::AssemblyMachine*>(targetProtoData_);
    assert(asm_machine);
    asm_machine->TryBeginCrafting(params.logicData, machine_data);

    return {true, {product_stack.item, params.amount}};
}


bool game::InserterPickup::IsAtMaxDegree(const proto::RotationDegreeT& degree) {
    return degree == proto::RotationDegreeT(kMaxInserterDegree);
}

std::pair<bool, proto::LineDistT> game::InserterPickup::GetBeltPickupProps(const PickupParams& params) {
    auto& line_data = static_cast<proto::ConveyorData&>(params.uniqueData);

    bool use_line_left = false;
    switch (line_data.structure->direction) {
    case Orientation::up:
        switch (params.orientation) {
        case Orientation::down:
        case Orientation::left:
            use_line_left = true;
            break;

        default:
            break;
        }
        break;

    case Orientation::right:
        switch (params.orientation) {
        case Orientation::up:
        case Orientation::left:
            use_line_left = true;
            break;

        default:
            break;
        }
        break;

    case Orientation::down:
        switch (params.orientation) {
        case Orientation::up:
        case Orientation::right:
            use_line_left = true;
            break;

        default:
            break;
        }
        break;

    case Orientation::left:
        switch (params.orientation) {
        case Orientation::right:
        case Orientation::down:
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

    auto pickup_offset = proto::LineDistT(
        line_data.structIndex +
        GetInserterArmOffset(core::SafeCast<core::TIntDegree>(params.degree.getAsInteger()), params.inserterTileReach));

    return {use_line_left, pickup_offset};
}
