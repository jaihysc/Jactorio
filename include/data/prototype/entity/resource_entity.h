// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_RESOURCE_ENTITY_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_RESOURCE_ENTITY_H
#pragma once

#include "data/prototype/entity/entity.h"

#include "core/data_type.h"

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

		Sprite::SetT OnRGetSet(Orientation,
		                       game::WorldData&,
		                       const game::WorldData::WorldPair&) const override {
			return 0;
		}


		void OnBuild(game::WorldData&,
		             game::LogicData&,
		             const game::WorldData::WorldPair&,
		             game::ChunkTileLayer&, Orientation) const override {
			assert(false);  // Is not player placeable
		}

		void OnRemove(game::WorldData&,
		              game::LogicData&,
		              const game::WorldData::WorldPair&, game::ChunkTileLayer&) const override {
		}


		void PostLoadValidate(const PrototypeManager&) const override {
			// Must convert to at least 1 game tick
			J_DATA_ASSERT(pickupTime * kGameHertz >= 1, "Pickup time is too small");
		}
	};
}

#endif //JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_RESOURCE_ENTITY_H
