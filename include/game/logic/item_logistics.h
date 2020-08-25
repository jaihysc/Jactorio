// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_GAME_LOGIC_ITEM_LOGISTICS_H
#define JACTORIO_GAME_LOGIC_ITEM_LOGISTICS_H
#pragma once

#include "data/prototype/assembly_machine.h"
#include "data/prototype/item.h"
#include "data/prototype/type.h"
#include "data/prototype/abstract_proto/transport_line.h"
#include "game/logic/logic_data.h"
#include "game/world/world_data.h"

namespace jactorio::game
{
	///
	/// Initialize: Return false if failed
#define J_ITEM_HANDLER_COMMON()                                                  \
	bool Initialize(const WorldData& world_data,                                 \
	                data::UniqueDataBase& target_unique_data,                    \
	                WorldCoordAxis world_x, WorldCoordAxis world_y);             \
																		         \
	bool Initialize(const WorldData& world_data,                                 \
	                data::UniqueDataBase& target_unique_data,                    \
	                const WorldCoord& world_coord) {                             \
						return Initialize(world_data, target_unique_data, world_coord.x, world_coord.y);};

	///
	/// \brief Base class for handling items (pickup / droOff)
	class ItemHandler
	{
	protected:
		///
		/// \param orientation Orientation from origin prototype, "the destination is <orientation> of prototype"
		explicit ItemHandler(const data::Orientation orientation)
			: orientation_(orientation) {
		}

	public:
		void Uninitialize() noexcept {
			targetUniqueData_ = nullptr;
		}

		J_NODISCARD bool IsInitialized() const noexcept {
			return targetUniqueData_ != nullptr;
		}


		J_NODISCARD data::Orientation GetOrientation() const noexcept {
			return orientation_;
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

		J_ITEM_HANDLER_COMMON()


		struct DropOffParams
		{
			LogicData& logicData;
			const data::ItemStack& itemStack;
			/// Entity to drop into
			data::UniqueDataBase& uniqueData;
			data::Orientation orientation;
		};

		///
		///	 \brief Insert provided item at destination
		bool DropOff(LogicData& logic_data, const data::ItemStack& item_stack) const {
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

		J_NODISCARD bool CanInsertContainerEntity(const DropOffParams& args) const;
		bool InsertContainerEntity(const DropOffParams& args) const;

		J_NODISCARD bool CanInsertTransportBelt(const DropOffParams& args) const;
		bool InsertTransportBelt(const DropOffParams& args) const;

		J_NODISCARD bool CanInsertAssemblyMachine(const DropOffParams& args) const;
		bool InsertAssemblyMachine(const DropOffParams& args) const;


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
		using PickupReturn = std::pair<bool, data::ItemStack>;
		using GetPickupReturn = const data::Item*;

	public:
		explicit InserterPickup(const data::Orientation orientation)
			: ItemHandler(orientation) {
		}

		J_ITEM_HANDLER_COMMON()

		/// \remark Picks up items when at max deg
		struct PickupParams
		{
			LogicData& logicData;
			data::ProtoUintT inserterTileReach;
			const data::RotationDegreeT& degree;
			data::Item::StackCount amount;
			data::UniqueDataBase& uniqueData;
			data::Orientation orientation;
		};

		///
		///	 \brief Insert provided item at destination
		PickupReturn Pickup(LogicData& logic_data,
		                    const data::ProtoUintT inserter_tile_reach,
		                    const data::RotationDegreeT& degree,
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
		                                      const data::RotationDegreeT& degree) const {
			assert(targetUniqueData_);
			assert(getPickupFunc_);
			return (this->*getPickupFunc_)({
				logic_data,
				inserter_tile_reach, degree, 1, *targetUniqueData_, orientation_
			});
		}

	protected:
		J_NODISCARD GetPickupReturn GetPickupContainerEntity(const PickupParams& args) const;
		PickupReturn PickupContainerEntity(const PickupParams& args) const;

		J_NODISCARD GetPickupReturn GetPickupTransportBelt(const PickupParams& args) const;
		PickupReturn PickupTransportBelt(const PickupParams& args) const;

		J_NODISCARD GetPickupReturn GetPickupAssemblyMachine(const PickupParams& args) const;
		PickupReturn PickupAssemblyMachine(const PickupParams& args) const;

		///
		/// \returns true if at maximum inserter degree
		static bool IsAtMaxDegree(const data::RotationDegreeT& degree);

		using PickupFunc = decltype(&InserterPickup::PickupContainerEntity);
		using GetPickupFunc = decltype(&InserterPickup::GetPickupContainerEntity);

		PickupFunc pickupFunc_       = nullptr;
		GetPickupFunc getPickupFunc_ = nullptr;

	private:
		static std::pair<bool, data::LineDistT> GetBeltPickupProps(const PickupParams& args);
	};

#undef J_ITEM_HANDLER_COMMON
}

#endif // JACTORIO_GAME_LOGIC_ITEM_LOGISTICS_H
