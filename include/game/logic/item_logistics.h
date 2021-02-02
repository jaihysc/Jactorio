// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_LOGIC_ITEM_LOGISTICS_H
#define JACTORIO_INCLUDE_GAME_LOGIC_ITEM_LOGISTICS_H
#pragma once

#include "core/coordinate_tuple.h"
#include "core/data_type.h"
#include "core/orientation.h"
#include "proto/detail/type.h"
#include "proto/item.h"

namespace jactorio::game
{
    class LogicData;
    class World;

    ///
    /// Initialize: Return false if failed
#define J_ITEM_HANDLER_COMMON                                                      \
    bool Initialize(World& world, WorldCoordAxis world_x, WorldCoordAxis world_y); \
                                                                                   \
    bool Initialize(World& world, const WorldCoord& coord) {                 \
        return Initialize(world, coord.x, coord.y);                    \
    };

    ///
    /// Base class for handling items (pickup / droOff)
    class ItemHandler
    {
    protected:
        ///
        /// \param orientation Orientation from origin prototype, "the destination is <orientation> of prototype"
        explicit ItemHandler(const Orientation orientation) : orientation_(orientation) {}

    public:
        void Uninitialize() noexcept {
            targetUniqueData_ = nullptr;
        }

        J_NODISCARD bool IsInitialized() const noexcept {
            return targetUniqueData_ != nullptr;
        }


        J_NODISCARD Orientation GetOrientation() const noexcept {
            return orientation_;
        }

    protected:
        proto::UniqueDataBase* targetUniqueData_     = nullptr;
        const proto::FrameworkBase* targetProtoData_ = nullptr;

        Orientation orientation_;
    };


    ///
    /// Represents a world location where items can be inserted
    class ItemDropOff : public ItemHandler
    {
    public:
        explicit ItemDropOff(const Orientation orientation) : ItemHandler(orientation) {}

        J_ITEM_HANDLER_COMMON


        struct DropOffParams
        {
            LogicData& logicData;
            const proto::ItemStack& itemStack;
            /// Entity to drop into
            proto::UniqueDataBase& uniqueData;
            Orientation orientation;
        };

        ///
        ///	 \brief Insert provided item at destination
        bool DropOff(LogicData& logic_data, const proto::ItemStack& item_stack) const {
            assert(targetUniqueData_);
            assert(dropFunc_);
            return (this->*dropFunc_)({logic_data, item_stack, *targetUniqueData_, orientation_});
        }

        ///
        ///	 \return true if dropoff can ever possible at the specified location
        J_NODISCARD bool CanDropOff(LogicData& logic_data, const proto::Item*& item) const {
            assert(targetUniqueData_);
            assert(canDropFunc_);
            return (this->*canDropFunc_)({logic_data, {item, 0}, *targetUniqueData_, orientation_});
        }

    protected:
        // Dropoff functions

        J_NODISCARD bool CanInsertContainerEntity(const DropOffParams& params) const;
        bool InsertContainerEntity(const DropOffParams& params) const;

        J_NODISCARD bool CanInsertTransportBelt(const DropOffParams& params) const;
        bool InsertTransportBelt(const DropOffParams& params) const;

        J_NODISCARD bool CanInsertAssemblyMachine(const DropOffParams& params) const;
        bool InsertAssemblyMachine(const DropOffParams& params) const;


        using DropOffFunc    = decltype(&ItemDropOff::InsertContainerEntity);
        using CanDropOffFunc = decltype(&ItemDropOff::CanInsertContainerEntity);

        /// Chosen function for inserting at destination
        DropOffFunc dropFunc_       = nullptr;
        CanDropOffFunc canDropFunc_ = nullptr;
    };

    ///
    /// Represents a world location where items can be picked up by inserters
    class InserterPickup : public ItemHandler
    {
        /// Success, picked up stack
        using PickupReturn    = std::pair<bool, proto::ItemStack>;
        using GetPickupReturn = const proto::Item*;

    public:
        explicit InserterPickup(const Orientation orientation) : ItemHandler(orientation) {}

        J_ITEM_HANDLER_COMMON

        /// \remark Picks up items when at max deg
        struct PickupParams
        {
            LogicData& logicData;
            proto::ProtoUintT inserterTileReach;
            const proto::RotationDegreeT& degree;
            proto::Item::StackCount amount;
            proto::UniqueDataBase& uniqueData;
            Orientation orientation;
        };

        ///
        ///	 \brief Insert provided item at destination
        PickupReturn Pickup(LogicData& logic_data,
                            const proto::ProtoUintT inserter_tile_reach,
                            const proto::RotationDegreeT& degree,
                            const proto::Item::StackCount amount) const {
            assert(targetUniqueData_);
            assert(pickupFunc_);
            return (this->*pickupFunc_)(
                {logic_data, inserter_tile_reach, degree, amount, *targetUniqueData_, orientation_});
        }

        ///
        /// \return Item which will picked up by Pickup()
        J_NODISCARD GetPickupReturn GetPickup(LogicData& logic_data,
                                              const proto::ProtoUintT inserter_tile_reach,
                                              const proto::RotationDegreeT& degree) const {
            assert(targetUniqueData_);
            assert(getPickupFunc_);
            return (this->*getPickupFunc_)(
                {logic_data, inserter_tile_reach, degree, 1, *targetUniqueData_, orientation_});
        }

    protected:
        J_NODISCARD GetPickupReturn GetPickupContainerEntity(const PickupParams& params) const;
        PickupReturn PickupContainerEntity(const PickupParams& params) const;

        J_NODISCARD GetPickupReturn GetPickupTransportBelt(const PickupParams& params) const;
        PickupReturn PickupTransportBelt(const PickupParams& params) const;

        J_NODISCARD GetPickupReturn GetPickupAssemblyMachine(const PickupParams& params) const;
        PickupReturn PickupAssemblyMachine(const PickupParams& params) const;

        ///
        /// \returns true if at maximum inserter degree
        static bool IsAtMaxDegree(const proto::RotationDegreeT& degree);

        using PickupFunc    = decltype(&InserterPickup::PickupContainerEntity);
        using GetPickupFunc = decltype(&InserterPickup::GetPickupContainerEntity);

        PickupFunc pickupFunc_       = nullptr;
        GetPickupFunc getPickupFunc_ = nullptr;

    private:
        static std::pair<bool, proto::LineDistT> GetBeltPickupProps(const PickupParams& params);
    };

#undef J_ITEM_HANDLER_COMMON
} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_LOGIC_ITEM_LOGISTICS_H
