// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 06/30/2020

#ifndef JACTORIO_DATA_PROTOTYPE_ENTITY_ASSEMBLY_MACHINE_H
#define JACTORIO_DATA_PROTOTYPE_ENTITY_ASSEMBLY_MACHINE_H
#pragma once

#include "data/prototype/type.h"
#include "data/prototype/entity/health_entity.h"

namespace jactorio::data
{
	struct AssemblyMachineData final : HealthEntityData
	{
		/// Currently selected recipe for assembling
		Recipe* recipe = nullptr;
	};

	class AssemblyMachine final : public HealthEntity
	{
	public:
		PROTOTYPE_CATEGORY(assembly_machine);

		/// Original crafting time / assemblySpeed = new crafting time
		PYTHON_PROP_REF_I(AssemblyMachine, ProtoFloatT, assemblySpeed, 1.f);

		// ======================================================================

		Sprite::SetT OnRGetSet(Orientation orientation, game::WorldData& world_data,
		                       const game::WorldData::WorldPair& world_coords) const override;

		SpritemapFrame OnRGetSprite(const UniqueDataBase* unique_data, GameTickT game_tick) const override;


		bool OnRShowGui(game::PlayerData& player_data, const DataManager& data_manager,
		                game::ChunkTileLayer* tile_layer) const override;

		void OnBuild(game::WorldData& world_data,
		             const game::WorldData::WorldPair& world_coords,
		             game::ChunkTileLayer& tile_layer, Orientation orientation) const override;

		void OnRemove(game::WorldData& world_data,
		              const game::WorldData::WorldPair& world_coords,
		              game::ChunkTileLayer& tile_layer) const override;

		void PostLoadValidate(const DataManager&) const override {
			J_DATA_ASSERT(assemblySpeed > 0.f, "Assembly speed cannot be 0");
		}
	};
}

#endif // JACTORIO_DATA_PROTOTYPE_ENTITY_ASSEMBLY_MACHINE_H
