// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 01/20/2020

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_CONTAINER_ENTITY_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_CONTAINER_ENTITY_H
#pragma once

#include <array>

#include "health_entity.h"

namespace jactorio::data
{
	struct ContainerEntityData : HealthEntityData
	{
		explicit ContainerEntityData(const uint16_t inventory_size)
			: inventory(new ItemStack[inventory_size]), size(inventory_size) {
		}

		~ContainerEntityData() override {
			delete[] inventory;
		}

		ContainerEntityData(const ContainerEntityData& other)                = delete;
		ContainerEntityData(ContainerEntityData&& other) noexcept            = delete;
		ContainerEntityData& operator=(const ContainerEntityData& other)     = delete;
		ContainerEntityData& operator=(ContainerEntityData&& other) noexcept = delete;

		ItemStack* const inventory;
		const uint16_t size;
	};

	///
	/// \brief An entity with an inventory, such as a chest
	class ContainerEntity final : public HealthEntity
	{
	public:
		PROTOTYPE_CATEGORY(container_entity);

		ContainerEntity()
			: inventorySize(0) {
		}

		PYTHON_PROP_REF(ContainerEntity, uint16_t, inventorySize)


		UniqueDataBase* CopyUniqueData(UniqueDataBase* ptr) const override;

		// Events

		void OnBuild(game::WorldData& world_data,
		             const game::WorldData::WorldPair& world_coords,
		             game::ChunkTileLayer& tile_layer,
		             Orientation orientation) const override;

		void OnRemove(game::WorldData&,
		              const game::WorldData::WorldPair&,
		              game::ChunkTileLayer&) const override {
		}

		void OnRShowGui(game::PlayerData& player_data, game::ChunkTileLayer* tile_layer) const override;

		std::pair<uint16_t, uint16_t> MapPlacementOrientation(Orientation,
		                                                      game::WorldData&,
		                                                      const game::WorldData::WorldPair&) const override {
			return {0, 0};
		}

		void ValidatedPostLoad() override {
			sprite->DefaultSpriteGroup({Sprite::SpriteGroup::terrain});
		}
	};
}

#endif //JACTORIO_INCLUDE_DATA_PROTOTYPE_ENTITY_CONTAINER_ENTITY_H
