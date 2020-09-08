// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_ASSEMBLY_MACHINE_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_ASSEMBLY_MACHINE_H
#pragma once

#include "data/cereal/serialization_type.h"
#include "data/prototype/abstract_proto/health_entity.h"
#include "data/prototype/recipe.h"
#include "data/prototype/type.h"

namespace jactorio::data
{
    class CraftAddProduct;

    struct AssemblyMachineData final : HealthEntityData
    {
        J_NODISCARD bool HasRecipe() const {
            return recipe_ != nullptr;
        }
        J_NODISCARD const Recipe* GetRecipe() const {
            return recipe_.Get();
        }

        ///
        /// Changes recipe to provided recipe, nullptr for no recipe
        void ChangeRecipe(game::LogicData& logic_data, const PrototypeManager& data_manager, const Recipe* new_recipe);

        ///
        /// Checks if necessary ingredients are present to begin crafting
        /// \return true if recipe crafting has begun
        J_NODISCARD bool CanBeginCrafting() const;

        ///
        /// Deducts items from ingredient inventory equal to amount specified by recipe
        void CraftRemoveIngredients();

        ///
        /// Outputs recipe product to product inventory
        void CraftAddProduct();


        /// Callback called when recipe is finished crafting
        game::DeferralTimer::DeferralEntry deferralEntry;

        Item::Inventory ingredientInv;
        Item::Inventory productInv;


        CEREAL_SERIALIZE(archive) {
            archive(cereal::base_class<HealthEntityData>(this), deferralEntry, ingredientInv, productInv, recipe_);
        }

    private:
        /// Currently selected recipe for assembling
        SerialProtoPtr<const Recipe> recipe_ = nullptr;
    };


    class AssemblyMachine final : public HealthEntity
    {
    public:
        PROTOTYPE_CATEGORY(assembly_machine);

        /// Original crafting time / assemblySpeed = new crafting time
        PYTHON_PROP_REF_I(ProtoFloatT, assemblySpeed, 1.);

        // ======================================================================

        J_NODISCARD Sprite::FrameT OnRGetSpriteFrame(const UniqueDataBase& unique_data,
                                                     GameTickT game_tick) const override;

        bool OnRShowGui(GameWorlds& worlds,
                        game::LogicData& logic,
                        game::PlayerData& player,
                        const PrototypeManager& proto_manager,
                        game::ChunkTileLayer* tile_layer) const override;


        // ======================================================================

        ///
        /// Begins crafting if ingredients are met
        /// \return true if crafting has begun
        bool TryBeginCrafting(game::LogicData& logic_data, AssemblyMachineData& data) const;

        void OnDeferTimeElapsed(game::WorldData& world_data,
                                game::LogicData& logic_data,
                                UniqueDataBase* unique_data) const override;

        void OnBuild(game::WorldData& world_data,
                     game::LogicData& logic_data,
                     const WorldCoord& world_coords,
                     game::ChunkTileLayer& tile_layer,
                     Orientation orientation) const override;

        void OnRemove(game::WorldData& world_data,
                      game::LogicData& logic_data,
                      const WorldCoord& world_coords,
                      game::ChunkTileLayer& tile_layer) const override;

        void PostLoadValidate(const PrototypeManager&) const override {
            J_DATA_ASSERT(assemblySpeed > 0., "Assembly speed cannot be 0");
        }
    };
} // namespace jactorio::data

#endif // JACTORIO_INCLUDE_DATA_PROTOTYPE_ASSEMBLY_MACHINE_H
