// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/logic/inserter_controller.h"

#include <vector>

#include "game/logic/logic.h"
#include "game/world/world.h"
#include "proto/inserter.h"

using namespace jactorio;

double game::GetInserterArmOffset(const TIntDegree degree, const unsigned target_distance) {
    auto result = kInserterCenterOffset + target_distance - kInserterArmTileGap;
    result *= TanF(degree);
    result *= -1;
    result += kInserterCenterOffset;

    return result;
}

double game::GetInserterArmLength(const TIntDegree degree, const unsigned target_distance) {
    auto result = kInserterCenterOffset + target_distance - kInserterArmTileGap;
    result /= CosF(degree);
    result *= -1;

    return result;
}


// ======================================================================


// Inserters are updated in stages together so that logic order does not affect inserter behavior
// E.g Inserter1 drop off             , inserter2 picks up | 1 logic update
//     Inserter2 has nothing to pickup, inserter1 drop off | 1 logic update

struct InserterUpdateProps
{
    const proto::Inserter& proto;
    proto::InserterData& data;
};

using DropoffQueue = std::vector<InserterUpdateProps>;
using PickupQueue  = std::vector<InserterUpdateProps>;

/// Rotates inserters, queues inserters awaiting dropoff and pickup handling
void RotateInserters(DropoffQueue& dropoff_queue, PickupQueue& pickup_queue, const InserterUpdateProps& props) {
    using namespace game;

    assert(props.proto.rotationSpeed.getAsDouble() != 0);

    switch (props.data.status) {

    case proto::InserterData::Status::dropoff:
        props.data.rotationDegree -= props.proto.rotationSpeed;

        if (props.data.rotationDegree <= proto::RotationDegreeT(kMinInserterDegree)) {
            props.data.rotationDegree = 0; // Prevents underflow if the inserter sits idle for a long time
            dropoff_queue.push_back(props);
        }

        return;

    case proto::InserterData::Status::pickup:
        // Rotate the inserter
        props.data.rotationDegree += props.proto.rotationSpeed;

        if (props.data.rotationDegree > proto::RotationDegreeT(kMaxInserterDegree)) {
            props.data.rotationDegree = kMaxInserterDegree; // Prevents overflow
            pickup_queue.push_back(props);
        }

        return;

    default:
        assert(false);
    }
}

void ProcessInserterDropoff(const DropoffQueue& dropoff_queue, game::Logic& logic) {
    for (const auto& inserter_prop : dropoff_queue) {
        auto& inserter_data = inserter_prop.data;

        if (inserter_data.dropoff.DropOff(logic, inserter_data.heldItem)) {
            inserter_data.status = proto::InserterData::Status::pickup;
        }
    }
}

void ProcessInserterPickup(const PickupQueue& pickup_queue, game::Logic& logic) {
    for (const auto& inserter_prop : pickup_queue) {
        auto& inserter_data        = inserter_prop.data;
        const auto& inserter_proto = inserter_prop.proto;

        constexpr int pickup_amount = 1;

        const auto* to_be_picked_item =
            inserter_data.pickup.GetPickup(logic, inserter_proto.tileReach, inserter_data.rotationDegree);

        // Do not pick up item if it cannot be dropped off
        if (!inserter_data.dropoff.CanDropOff(logic, to_be_picked_item))
            continue;


        const auto result =
            inserter_data.pickup.Pickup(logic, inserter_proto.tileReach, inserter_data.rotationDegree, pickup_amount);
        if (result.first) {
            inserter_data.heldItem = result.second;

            inserter_data.status = proto::InserterData::Status::dropoff;
        }
    }
}

void game::InserterLogicUpdate(World& world, Logic& logic) {
    DropoffQueue dropoff_queue{}; // TODO We can allocate this ahead of time
    PickupQueue pickup_queue{};

    for (auto [prototype, unique_data, coord] : world.LogicGet(LogicGroup::inserter)) {
        const auto* inserter_proto = SafeCast<const proto::Inserter*>(prototype.Get());
        assert(inserter_proto != nullptr);

        auto* inserter_data = SafeCast<proto::InserterData*>(unique_data.Get());
        assert(inserter_data != nullptr);

        RotateInserters(dropoff_queue, pickup_queue, {*inserter_proto, *inserter_data});
    }

    ProcessInserterDropoff(dropoff_queue, logic);
    ProcessInserterPickup(pickup_queue, logic);
}
