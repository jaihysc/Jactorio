// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 02/07/2020

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_RESOURCE_ENTITY_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_RESOURCE_ENTITY_H
#pragma once

#include "data/prototype/entity/entity.h"

namespace jactorio::data
{
	// Unique per resource entity placed
	struct ResourceEntityData : EntityData
	{
		explicit ResourceEntityData(const uint16_t resource_amount)
			: resourceAmount(resource_amount) {
		}

		///
		/// \brief Amount of product which can still be extracted from this tile
		uint16_t resourceAmount;
	};

	class ResourceEntity final : public Entity
	{
	public:
		PROTOTYPE_CATEGORY(resource_entity);

		ResourceEntity() {
			// Resource entities can never be placed
			this->placeable = false;
		}

		UniqueDataBase* CopyUniqueData(UniqueDataBase* ptr) const override {
			return new ResourceEntityData(*static_cast<ResourceEntityData*>(ptr));
		}


		void OnRShowGui(game::PlayerData&, game::ChunkTileLayer*) const override {
		}

		Sprite::SetT MapPlacementOrientation(Orientation,
		                                     game::WorldData&,
		                                     const game::WorldData::WorldPair&) const override {
			return 0;
		}


		void OnBuild(game::WorldData&,
		             const game::WorldData::WorldPair&,
		             game::ChunkTileLayer&,
		             Orientation) const override {
			assert(false);  // Is not player placeable
		}

		void OnRemove(game::WorldData&,
		              const game::WorldData::WorldPair&,
		              game::ChunkTileLayer&) const override {
		}


		void PostLoadValidate() const override {
			// Must convert to at least 1 game tick
			J_DATA_ASSERT(pickupTime * JC_GAME_HERTZ >= 1, "Pickup time is too small");
		}
	};
}

#endif //JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_RESOURCE_ENTITY_H
