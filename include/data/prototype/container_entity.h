// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_CONTAINER_ENTITY_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_CONTAINER_ENTITY_H
#pragma once

#include "data/prototype/abstract_proto/health_entity.h"

namespace jactorio::data
{
	struct ContainerEntityData : HealthEntityData
	{
		explicit ContainerEntityData(const uint16_t inventory_size) {
			inventory.resize(inventory_size);
		}

		Item::Inventory inventory;
	};

	///
	/// \brief An entity with an inventory, such as a chest
	class ContainerEntity final : public HealthEntity
	{
	public:
		PROTOTYPE_CATEGORY(container_entity);
		PROTOTYPE_DATA_TRIVIAL_COPY(ContainerEntityData);

		ContainerEntity()
			: inventorySize(0) {
		}

		PYTHON_PROP_REF(uint16_t, inventorySize);


		// Events

		void OnBuild(game::WorldData& world_data,
		             game::LogicData& logic_data,
		             const WorldCoord& world_coords,
		             game::ChunkTileLayer& tile_layer, Orientation orientation) const override;

		void OnRemove(game::WorldData&,
		              game::LogicData&,
		              const WorldCoord&, game::ChunkTileLayer&) const override {
		}

		bool OnRShowGui(game::PlayerData& player_data, const PrototypeManager& data_manager,
		                game::ChunkTileLayer* tile_layer) const override;

		void ValidatedPostLoad() override {
			sprite->DefaultSpriteGroup({Sprite::SpriteGroup::terrain});
		}
	};
}

#endif //JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_CONTAINER_ENTITY_H
