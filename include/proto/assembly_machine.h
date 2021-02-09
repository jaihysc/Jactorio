// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_PROTO_ASSEMBLY_MACHINE_H
#define JACTORIO_INCLUDE_PROTO_ASSEMBLY_MACHINE_H
#pragma once

#include "data/cereal/serialization_type.h"
#include "game/logic/deferral_timer.h"
#include "game/logistic/inventory.h"
#include "proto/abstract/health_entity.h"
#include "proto/recipe.h"

namespace jactorio::proto
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
        void ChangeRecipe(game::Logic& logic, const data::PrototypeManager& proto, const Recipe* new_recipe);

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

        game::Inventory ingredientInv;
        game::Inventory productInv;


        CEREAL_SERIALIZE(archive) {
            archive(cereal::base_class<HealthEntityData>(this), deferralEntry, ingredientInv, productInv, recipe_);
        }

    private:
        /// Currently selected recipe for assembling
        data::SerialProtoPtr<const Recipe> recipe_ = nullptr;
    };


    class AssemblyMachine final : public HealthEntity
    {
    public:
        PROTOTYPE_CATEGORY(assembly_machine);
        PROTOTYPE_DATA_TRIVIAL_COPY(AssemblyMachineData);

        /// Original crafting time / assemblySpeed = new crafting time
        PYTHON_PROP_REF_I(ProtoFloatT, assemblySpeed, 1.);

        // ======================================================================

        J_NODISCARD SpriteFrameT OnRGetSpriteFrame(const UniqueDataBase& unique_data,
                                                   GameTickT game_tick) const override;


        bool OnRShowGui(const render::GuiRenderer& g_rendr, game::ChunkTileLayer* tile_layer) const override;


        // ======================================================================

        ///
        /// Begins crafting if ingredients are met
        /// \return true if crafting has begun
        bool TryBeginCrafting(game::Logic& logic, AssemblyMachineData& data) const;

        void OnDeferTimeElapsed(game::World& world, game::Logic& logic, UniqueDataBase* unique_data) const override;

        void OnBuild(game::World& world,
                     game::Logic& logic,
                     const WorldCoord& coord,
                     game::ChunkTileLayer& tile_layer,
                     Orientation orientation) const override;

        void OnRemove(game::World& world,
                      game::Logic& logic,
                      const WorldCoord& coord,
                      game::ChunkTileLayer& tile_layer) const override;

        void PostLoadValidate(const data::PrototypeManager& /*proto*/) const override {
            J_PROTO_ASSERT(assemblySpeed > 0., "Assembly speed cannot be 0");
        }
    };
} // namespace jactorio::proto

#endif // JACTORIO_INCLUDE_PROTO_ASSEMBLY_MACHINE_H
