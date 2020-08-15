// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_GAME_LOGIC_ITEM_LOGISTICS_H
#define JACTORIO_GAME_LOGIC_ITEM_LOGISTICS_H
#pragma once

#include "data/prototype/type.h"
#include "data/prototype/assembly_machine.h"
#include "data/prototype/abstract_proto/transport_line.h"
#include "data/prototype/item.h"
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
		const data::FrameworkBase* targetProtoData_ = nullptr;

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


		struct DropOffParams
		{
			LogicData& logicData;
			const data::Item::Stack& itemStack;
			/// Entity to drop into
			data::UniqueDataBase& uniqueData;
			data::Orientation orientation;
		};

		///
		///	 \brief Insert provided item at destination
		bool DropOff(LogicData& logic_data, const data::Item::Stack& item_stack) const {
			assert(targetUniqueData_);
			assert(dropFunc_);
			return (this->*dropFunc_)({logic_data, item_stack, *targetUniqueData_, orientation_});
		}

		///
		///	 \return true if dropoff can ever possible at the specified location 
		J_NODISCARD bool CanDropOff(LogicData& logic_data, const data::Item*& item) const {
			assert(targetUniqueData_);
			assert(canDropFunc_);
			return (this->*canDropFunc_)({logic_data, {item, 0}, *targetUniqueData_, orientation_});
		}

	protected:
		// Dropoff functions

		J_NODISCARD virtual bool CanInsertContainerEntity(const DropOffParams& args) const;
		virtual bool InsertContainerEntity(const DropOffParams& args) const;

		J_NODISCARD virtual bool CanInsertTransportBelt(const DropOffParams& args) const;
		virtual bool InsertTransportBelt(const DropOffParams& args) const;

		J_NODISCARD virtual bool CanInsertAssemblyMachine(const DropOffParams& args) const;
		virtual bool InsertAssemblyMachine(const DropOffParams& args) const;


		using DropOffFunc = decltype(&ItemDropOff::InsertContainerEntity);
		using CanDropOffFunc = decltype(&ItemDropOff::CanInsertContainerEntity);

		/// \brief Chosen function for inserting at destination
		DropOffFunc dropFunc_       = nullptr;
		CanDropOffFunc canDropFunc_ = nullptr;
	};

	///
	/// \brief Represents a world location where items can be picked up by inserters
	class InserterPickup : public ItemHandler
	{
		/// Success, picked up stack
		using PickupReturn = std::pair<bool, data::Item::Stack>;
		using GetPickupReturn = const data::Item*;

	public:
		explicit InserterPickup(const data::Orientation orientation)
			: ItemHandler(orientation) {
		}

		bool Initialize(const WorldData& world_data,
		                data::UniqueDataBase& target_unique_data,
		                WorldCoordAxis world_x, WorldCoordAxis world_y) override;

		bool Initialize(const WorldData& world_data,
		                data::UniqueDataBase& target_unique_data, const WorldCoord& world_coord) override;


		/// \remark Picks up items when at max deg
		struct PickupParams
		{
			LogicData& logicData;
			data::ProtoUintT inserterTileReach;
			const data::RotationDegree& degree;
			data::Item::StackCount amount;
			data::UniqueDataBase& uniqueData;
			data::Orientation orientation;
		};

		///
		///	 \brief Insert provided item at destination
		PickupReturn Pickup(LogicData& logic_data,
		                    const data::ProtoUintT inserter_tile_reach,
		                    const data::RotationDegree& degree,
		                    const data::Item::StackCount amount) const {
			assert(targetUniqueData_);
			assert(pickupFunc_);
			return (this->*pickupFunc_)({
				logic_data,
				inserter_tile_reach, degree, amount, *targetUniqueData_, orientation_
			});
		}

		///
		/// \return Item which will picked up by Pickup()
		J_NODISCARD GetPickupReturn GetPickup(LogicData& logic_data,
		                                      const data::ProtoUintT inserter_tile_reach,
		                                      const data::RotationDegree& degree) const {
			assert(targetUniqueData_);
			assert(getPickupFunc_);
			return (this->*getPickupFunc_)({
				logic_data,
				inserter_tile_reach, degree, 1, *targetUniqueData_, orientation_
			});
		}

	protected:
		J_NODISCARD virtual GetPickupReturn GetPickupContainerEntity(const PickupParams& args) const;
		virtual PickupReturn PickupContainerEntity(const PickupParams& args) const;

		J_NODISCARD virtual GetPickupReturn GetPickupTransportBelt(const PickupParams& args) const;
		virtual PickupReturn PickupTransportBelt(const PickupParams& args) const;

		J_NODISCARD virtual GetPickupReturn GetPickupAssemblyMachine(const PickupParams& args) const;
		virtual PickupReturn PickupAssemblyMachine(const PickupParams& args) const;

		///
		/// \returns true if at maximum inserter degree
		static bool IsAtMaxDegree(const data::RotationDegree& degree);

		using PickupFunc = decltype(&InserterPickup::PickupContainerEntity);
		using GetPickupFunc = decltype(&InserterPickup::GetPickupContainerEntity);

		PickupFunc pickupFunc_       = nullptr;
		GetPickupFunc getPickupFunc_ = nullptr;

	private:
		static std::pair<bool, TransportLineOffset> GetBeltPickupProps(const PickupParams& args);
	};
}

#endif // JACTORIO_GAME_LOGIC_ITEM_LOGISTICS_H
