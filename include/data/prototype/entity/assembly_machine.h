// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 06/30/2020

#ifndef JACTORIO_DATA_PROTOTYPE_ENTITY_ASSEMBLY_MACHINE_H
#define JACTORIO_DATA_PROTOTYPE_ENTITY_ASSEMBLY_MACHINE_H
#pragma once

#include "data/prototype/type.h"
#include "data/prototype/entity/health_entity.h"
#include "data/prototype/interface/deferred.h"
#include "game/world/world_data.h"

namespace jactorio::data
{
	class AssemblyMachine;
	
	struct AssemblyMachineData final : HealthEntityData
	{
		// Holds input / output items for assembling
		
		Item::Inventory ingredientInv;
		Item::Inventory productInv;
		
		/// Callback called when recipe is finished crafting
		game::LogicData::DeferralTimer::DeferralEntry deferralEntry;

		
		J_NODISCARD bool HasRecipe() const { return recipe_ != nullptr; }
		J_NODISCARD const Recipe* GetRecipe() const { return recipe_; }

		///
		/// \brief Changes recipe to provided recipe, nullptr for no recipe
		void ChangeRecipe(game::LogicData& logic_data, const PrototypeManager& data_manager,
		                  const AssemblyMachine& assembly_proto, const Recipe* new_recipe);

	private:
		/// Currently selected recipe for assembling
		const Recipe* recipe_ = nullptr;
	};


	class AssemblyMachine final : public HealthEntity, public IDeferred
	{
	public:
		PROTOTYPE_CATEGORY(assembly_machine);

		/// Original crafting time / assemblySpeed = new crafting time
		PYTHON_PROP_REF_I(AssemblyMachine, ProtoFloatT, assemblySpeed, 1.f);

		// ======================================================================

		Sprite::SetT OnRGetSet(Orientation orientation, game::WorldData& world_data,
		                       const game::WorldData::WorldPair& world_coords) const override;

		SpritemapFrame OnRGetSprite(const UniqueDataBase* unique_data, GameTickT game_tick) const override;


		bool OnRShowGui(game::PlayerData& player_data, const PrototypeManager& data_manager,
		                game::ChunkTileLayer* tile_layer) const override;


		void OnDeferTimeElapsed(game::WorldData& world_data, game::LogicData& logic_data, UniqueDataBase* unique_data) const override;

		void OnBuild(game::WorldData& world_data,
		             game::LogicData& logic_data,
		             const game::WorldData::WorldPair& world_coords, game::ChunkTileLayer& tile_layer, Orientation orientation) const override;

		void OnRemove(game::WorldData& world_data,
		              game::LogicData& logic_data,
		              const game::WorldData::WorldPair& world_coords, game::ChunkTileLayer& tile_layer) const override;

		void PostLoadValidate(const PrototypeManager&) const override {
			J_DATA_ASSERT(assemblySpeed > 0.f, "Assembly speed cannot be 0");
		}
	};
}

#endif // JACTORIO_DATA_PROTOTYPE_ENTITY_ASSEMBLY_MACHINE_H
