// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_DATA_PROTOTYPE_ENTITY_ASSEMBLY_MACHINE_H
#define JACTORIO_DATA_PROTOTYPE_ENTITY_ASSEMBLY_MACHINE_H
#pragma once

#include "data/prototype/type.h"
#include "data/prototype/entity/health_entity.h"
#include "data/prototype/interface/deferred.h"
#include "data/prototype/item/recipe.h"

namespace jactorio::data
{
	class CraftAddProduct;

	struct AssemblyMachineData final : HealthEntityData
	{
		Item::Inventory ingredientInv;
		Item::Inventory productInv;

		/// Callback called when recipe is finished crafting
		game::LogicData::DeferralTimer::DeferralEntry deferralEntry;


		J_NODISCARD bool HasRecipe() const { return recipe_ != nullptr; }
		J_NODISCARD const Recipe* GetRecipe() const { return recipe_; }

		///
		/// \brief Changes recipe to provided recipe, nullptr for no recipe
		void ChangeRecipe(game::LogicData& logic_data, const PrototypeManager& data_manager,
		                  const Recipe* new_recipe);

		///
		/// \brief Checks if necessary ingredients are present to begin crafting
		/// \return true if recipe crafting has begun
		J_NODISCARD bool CanBeginCrafting() const;

		///
		/// \brief Deducts items from ingredient inventory equal to amount specified by recipe
		void CraftRemoveIngredients();

		///
		/// \brief Outputs recipe product to product inventory
		void CraftAddProduct();

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

		J_NODISCARD Sprite::FrameT OnRGetSpriteFrame(const UniqueDataBase& unique_data, GameTickT game_tick) const override;

		bool OnRShowGui(game::PlayerData& player_data, const PrototypeManager& data_manager,
		                game::ChunkTileLayer* tile_layer) const override;


		// ======================================================================

		///
		/// \brief Begins crafting if ingredients are met
		/// \return true if crafting has begun
		bool TryBeginCrafting(game::LogicData& logic_data,
		                      AssemblyMachineData& data) const;

		void OnDeferTimeElapsed(game::WorldData& world_data, game::LogicData& logic_data,
		                        UniqueDataBase* unique_data) const override;

		void OnBuild(game::WorldData& world_data,
		             game::LogicData& logic_data,
		             const WorldCoord& world_coords, game::ChunkTileLayer& tile_layer,
		             Orientation orientation) const override;

		void OnRemove(game::WorldData& world_data,
		              game::LogicData& logic_data,
		              const WorldCoord& world_coords, game::ChunkTileLayer& tile_layer) const override;

		void PostLoadValidate(const PrototypeManager&) const override {
			J_DATA_ASSERT(assemblySpeed > 0.f, "Assembly speed cannot be 0");
		}
	};
}

#endif // JACTORIO_DATA_PROTOTYPE_ENTITY_ASSEMBLY_MACHINE_H
