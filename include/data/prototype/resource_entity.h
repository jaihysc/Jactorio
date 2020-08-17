// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_RESOURCE_ENTITY_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_RESOURCE_ENTITY_H
#pragma once

#include "data/prototype/abstract_proto/entity.h"

#include "core/data_type.h"

namespace jactorio::data
{
	// Unique per resource entity placed
	struct ResourceEntityData final : EntityData
	{
		using ResourceCount = uint32_t;

		/// Resource entity should never have 0 resources, when it does it is treated as infinite
		static constexpr ResourceCount kInfiniteResource = 0;


		explicit ResourceEntityData(const ResourceCount resource_amount)
			: resourceAmount(resource_amount) {
		}

		///
		/// \brief Amount of product which can still be extracted from this tile
		ResourceCount resourceAmount;
	};

	class ResourceEntity final : public Entity
	{
	public:
		PROTOTYPE_CATEGORY(resource_entity);

		ResourceEntity() {
			// Resource entities can never be placed
			this->placeable = false;
		}

		std::unique_ptr<UniqueDataBase> CopyUniqueData(UniqueDataBase* ptr) const override {
			return std::make_unique<ResourceEntityData>(*static_cast<ResourceEntityData*>(ptr));
		}

		void OnBuild(game::WorldData&,
		             game::LogicData&,
		             const WorldCoord&,
		             game::ChunkTileLayer&, Orientation) const override {
			assert(false);  // Is not player placeable
		}

		void OnRemove(game::WorldData&,
		              game::LogicData&,
		              const WorldCoord&, game::ChunkTileLayer&) const override {
		}


		void PostLoadValidate(const PrototypeManager&) const override {
			// Must convert to at least 1 game tick
			J_DATA_ASSERT(pickupTime * kGameHertz >= 1, "Pickup time is too small");
		}
	};
}

#endif //JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_RESOURCE_ENTITY_H
