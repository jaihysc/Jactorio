// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_GAME_LOGIC_ITEM_LOGISTICS_H
#define JACTORIO_GAME_LOGIC_ITEM_LOGISTICS_H
#pragma once

#include "data/prototype/type.h"
#include "data/prototype/entity/assembly_machine.h"
#include "data/prototype/item/item.h"
#include "game/logic/logic_data.h"
#include "game/world/world_data.h"

namespace jactorio::game
{
	///
	/// \brief Abstract base class for handling items (pickup / droOff)
	class ItemHandler
	{
	protected:
		///
		/// \param orientation Orientation from origin prototype, "the destination is <orientation> of prototype"
		explicit ItemHandler(const data::Orientation orientation)
			: orientation_(orientation) {
		}

		virtual ~ItemHandler() = default;

	public:
		///
		/// \brief Sets up function for handling items
		/// \return false if failed
		virtual bool Initialize(const WorldData& world_data,
		                        data::UniqueDataBase& target_unique_data,
		                        WorldCoordAxis world_x, WorldCoordAxis world_y) = 0;

		virtual bool Initialize(const WorldData& world_data,
		                        data::UniqueDataBase& target_unique_data, const WorldCoord& world_coord) = 0;

		virtual void Uninitialize() {
			targetUniqueData_ = nullptr;
		}

		J_NODISCARD virtual bool IsInitialized() {
			return targetUniqueData_ != nullptr;
		}

	protected:
		data::UniqueDataBase* targetUniqueData_     = nullptr;
		const data::PrototypeBase* targetProtoData_ = nullptr;

		data::Orientation orientation_;
	};


	///
	/// \brief Represents a world location where items can be inserted
	class ItemDropOff : public ItemHandler
	{
	public:
		explicit ItemDropOff(const data::Orientation orientation)
			: ItemHandler(orientation) {
		}

		bool Initialize(const WorldData& world_data,
		                data::UniqueDataBase& target_unique_data,
		                WorldCoordAxis world_x, WorldCoordAxis world_y) override;

		bool Initialize(const WorldData& world_data,
		                data::UniqueDataBase& target_unique_data, const WorldCoord& world_coord) override;

		///
		///	 \brief Insert provided item at destination
		bool DropOff(LogicData& logic_data, const data::Item::Stack& item_stack) const {
			assert(targetUniqueData_);
			return (this->*dropFunc_)(logic_data, item_stack, *targetUniqueData_, orientation_);
		}

	protected:
		// Dropoff functions

		virtual bool InsertContainerEntity(LogicData& logic_data,
		                                   const data::Item::Stack& item_stack,
		                                   data::UniqueDataBase& unique_data,
		                                   data::Orientation orientation) const;

		virtual bool InsertTransportBelt(LogicData& logic_data,
		                                 const data::Item::Stack& item_stack,
		                                 data::UniqueDataBase& unique_data,
		                                 data::Orientation orientation) const;

		virtual bool InsertAssemblyMachine(LogicData& logic_data,
		                                   const data::Item::Stack& item_stack,
		                                   data::UniqueDataBase& unique_data,
		                                   data::Orientation orientation) const;

		using DropOffFunc = decltype(&ItemDropOff::InsertContainerEntity);

		/// \brief Chosen function for inserting at destination
		DropOffFunc dropFunc_ = nullptr;
	};

	///
	/// \brief Represents a world location where items can be picked up by inserters
	class InserterPickup : public ItemHandler
	{
		/// Success, picked up stack
		using PickupReturn = std::pair<bool, data::Item::Stack>;

	public:
		explicit InserterPickup(const data::Orientation orientation)
			: ItemHandler(orientation) {
		}

		bool Initialize(const WorldData& world_data,
		                data::UniqueDataBase& target_unique_data,
		                WorldCoordAxis world_x, WorldCoordAxis world_y) override;

		bool Initialize(const WorldData& world_data,
		                data::UniqueDataBase& target_unique_data, const WorldCoord& world_coord) override;

		///
		///	 \brief Insert provided item at destination
		PickupReturn Pickup(LogicData& logic_data,
		                    const data::ProtoUintT inserter_tile_reach,
		                    const data::RotationDegree& degree,
		                    const data::Item::StackCount amount) const {
			assert(targetUniqueData_);
			return (this->*pickupFunc_)(logic_data,
			                            inserter_tile_reach, degree, amount, *targetUniqueData_, orientation_);
		}

	protected:
		///
		/// \remark Picks up items when at max deg
		/// \param unique_data Unique data of container to be picked up from 
		virtual PickupReturn PickupContainerEntity(LogicData& logic_data,
		                                           data::ProtoUintT inserter_tile_reach,
		                                           const data::RotationDegree& degree,
		                                           data::Item::StackCount amount,
		                                           data::UniqueDataBase& unique_data,
		                                           data::Orientation orientation) const;

		///
		/// \remark Will only pickup 1 from transport lines regardless of amount
		/// \param unique_data Unique data of transport belt to be picked up from 
		virtual PickupReturn PickupTransportBelt(LogicData& logic_data,
		                                         data::ProtoUintT inserter_tile_reach,
		                                         const data::RotationDegree& degree,
		                                         data::Item::StackCount amount,
		                                         data::UniqueDataBase& unique_data,
		                                         data::Orientation orientation) const;

		///
		/// \remark Picks up items when at max deg
		/// \param unique_data Unique data of transport belt to be picked up from 
		virtual PickupReturn PickupAssemblyMachine(LogicData& logic_data,
		                                           data::ProtoUintT inserter_tile_reach,
		                                           const data::RotationDegree& degree,
		                                           data::Item::StackCount amount,
		                                           data::UniqueDataBase& unique_data,
		                                           data::Orientation orientation) const;

		///
		/// \returns true if at maximum inserter degree
		static bool IsAtMaxDegree(const data::RotationDegree& degree);

		using PickupFunc = decltype(&InserterPickup::PickupContainerEntity);

		/// \brief Function for inserting at destination, from one below
		PickupFunc pickupFunc_ = nullptr;
	};
}

#endif // JACTORIO_GAME_LOGIC_ITEM_LOGISTICS_H
