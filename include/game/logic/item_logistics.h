// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 04/07/2020

#ifndef JACTORIO_GAME_LOGIC_ITEM_LOGISTICS_H
#define JACTORIO_GAME_LOGIC_ITEM_LOGISTICS_H
#pragma once

#include "data/prototype/type.h"
#include "data/prototype/item/item.h"
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
		                        WorldData::WorldCoord world_x, WorldData::WorldCoord world_y) = 0;

		virtual bool Initialize(const WorldData& world_data,
		                        data::UniqueDataBase& target_unique_data, const WorldData::WorldPair& world_coord) = 0;

		virtual void Uninitialize() {
			targetUniqueData_ = nullptr;
		}

		J_NODISCARD virtual bool IsInitialized() {
			return targetUniqueData_ != nullptr;
		}

	protected:
		/// \brief Tile location where item will be inserted
		data::UniqueDataBase* targetUniqueData_ = nullptr;
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
		                WorldData::WorldCoord world_x, WorldData::WorldCoord world_y) override;

		bool Initialize(const WorldData& world_data,
		                data::UniqueDataBase& target_unique_data, const WorldData::WorldPair& world_coord) override;

		///
		///	 \brief Insert provided item at destination
		bool DropOff(const data::ItemStack& item_stack) const {
			assert(targetUniqueData_);
			return (this->*dropFunc_)(item_stack, *targetUniqueData_, orientation_);
		}

	protected:
		// Dropoff functions

		virtual bool InsertContainerEntity(const data::ItemStack& item_stack,
		                                   data::UniqueDataBase& unique_data,
		                                   data::Orientation orientation) const;

		virtual bool InsertTransportBelt(const data::ItemStack& item_stack,
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
	public:
		explicit InserterPickup(const data::Orientation orientation)
			: ItemHandler(orientation) {
		}

		bool Initialize(const WorldData& world_data,
		                data::UniqueDataBase& target_unique_data,
		                WorldData::WorldCoord world_x, WorldData::WorldCoord world_y) override;

		bool Initialize(const WorldData& world_data,
		                data::UniqueDataBase& target_unique_data, const WorldData::WorldPair& world_coord) override;

		///
		///	 \brief Insert provided item at destination
		bool Pickup(const data::RotationDegree& degree,
		            const data::ItemStack::second_type amount,
		            data::ItemStack& out_item_stack) const {
			assert(targetUniqueData_);
			return (this->*pickupFunc_)(degree, amount, *targetUniqueData_, orientation_, out_item_stack);
		}

	protected:
		///
		/// \brief Picks up items when at 180 deg
		/// \param unique_data Unique data of container to be picked up from 
		/// \param out_item_stack Item which was picked up
		virtual bool PickupContainerEntity(const data::RotationDegree& degree,
		                                   data::ItemStack::second_type amount,
		                                   data::UniqueDataBase& unique_data,
		                                   data::Orientation orientation,
		                                   data::ItemStack& out_item_stack) const;

		///
		/// \param unique_data Unique data of transport belt to be picked up from 
		/// \param out_item_stack Item which was picked up
		virtual bool PickupTransportBelt(const data::RotationDegree& degree,
		                                 data::ItemStack::second_type amount,
		                                 data::UniqueDataBase& unique_data,
		                                 data::Orientation orientation,
		                                 data::ItemStack& out_item_stack) const;

		using PickupFunc = decltype(&InserterPickup::PickupContainerEntity);

		/// \brief Function for inserting at destination, from one below
		PickupFunc pickupFunc_ = nullptr;
	};
}

#endif // JACTORIO_GAME_LOGIC_ITEM_LOGISTICS_H
