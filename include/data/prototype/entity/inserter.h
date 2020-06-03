// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 05/25/2020

#ifndef JACTORIO_DATA_PROTOTYPE_ENTITY_INSERTER_H
#define JACTORIO_DATA_PROTOTYPE_ENTITY_INSERTER_H
#pragma once

#include <decimal.h>

#include "data/prototype/type.h"
#include "data/prototype/entity/health_entity.h"
#include "game/logic/inserter_controller.h"
#include "game/logic/item_logistics.h"

namespace jactorio::data
{
	///
	/// \brief Holds the internal structure for inserters
	struct InserterData final : HealthEntityData
	{
		explicit InserterData(const Orientation orientation)
			: orientation(orientation), dropoff(orientation), pickup(orientation) {
		}

		enum class Status
		{
			dropoff,
			pickup
		};

		// Orientation points towards dropoff
		Orientation orientation;

		/// Rotation degree of current inserter, 0 is dropoff, 180 is pickup 
		RotationDegree rotationDegree = ToRotationDegree(game::kMaxInserterDegree);

		/// Current inserter status
		Status status = Status::pickup;

		/// Current item held by inserter
		ItemStack heldItem;

		game::ItemDropOff dropoff;
		game::InserterPickup pickup;
	};


	class Inserter final : public HealthEntity
	{
	public:
		PROTOTYPE_CATEGORY(inserter);

		///
		/// \brief Degrees to rotate per tick 
		/// \remark For Python API use only
		PYTHON_PROP_I(Inserter, double, rotationSpeedFloat, 0.1f);

		///
		/// \brief Degrees to rotate per tick 
		PYTHON_PROP_REF(Inserter, RotationDegree, rotationSpeed);


		void PostLoad() override {
			rotationSpeed = ToRotationDegree(rotationSpeedFloat);
		}


		// ======================================================================
		void OnRShowGui(game::PlayerData& player_data, game::ChunkTileLayer* tile_layer) const override;

		Sprite::SetT MapPlacementOrientation(Orientation orientation,
		                                     game::WorldData& world_data,
		                                     const game::WorldData::WorldPair& world_coords) const override;


		void OnBuild(game::WorldData& world_data, const game::WorldData::WorldPair& world_coords,
		             game::ChunkTileLayer& tile_layer, Orientation orientation) const override;

		void OnNeighborUpdate(game::WorldData& world_data,
		                      const game::WorldData::WorldPair& emit_world_coords,
		                      const game::WorldData::WorldPair& receive_world_coords,
		                      Orientation emit_orientation) const override;

		void OnRemove(game::WorldData& world_data, const game::WorldData::WorldPair& world_coords,
		              game::ChunkTileLayer& tile_layer) const override;

		void ValidatedPostLoad() override {
			sprite->DefaultSpriteGroup({Sprite::SpriteGroup::terrain});
		}
	};
}

#endif // JACTORIO_DATA_PROTOTYPE_ENTITY_INSERTER_H
