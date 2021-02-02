// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "proto/assembly_machine.h"

#include "game/logic/logic.h"
#include "game/world/world.h"
#include "gui/menus.h"
#include "proto/recipe.h"

using namespace jactorio;

void proto::AssemblyMachineData::ChangeRecipe(game::Logic& logic,
                                              const data::PrototypeManager& data_manager,
                                              const Recipe* new_recipe) {
    if (new_recipe != nullptr) {
        ingredientInv.resize(new_recipe->ingredients.size());
        productInv.resize(1);

        // Set filters
        for (size_t i = 0; i < ingredientInv.size(); ++i) {
            const auto* ingredient = data_manager.DataRawGet<Item>(new_recipe->ingredients[i].first);
            assert(ingredient != nullptr);
            ingredientInv[i].filter = ingredient;
        }

        const auto* product = data_manager.DataRawGet<Item>(new_recipe->product.first);
        assert(product);
        productInv[0].filter = product;
    }
    else {
        // Remove recipe
        logic.deferralTimer.RemoveDeferralEntry(deferralEntry);

        ingredientInv.resize(0);
        productInv.resize(0);
    }

    recipe_ = new_recipe;
}

bool proto::AssemblyMachineData::CanBeginCrafting() const {
    if (recipe_ == nullptr)
        return false;

    // Cannot craft if it will exceed stacksize
    if (productInv[0].item != nullptr && productInv[0].count + recipe_->product.second > productInv[0].item->stackSize)
        return false;

    for (size_t i = 0; i < recipe_->ingredients.size(); ++i) {
        const auto& i_possessed = ingredientInv[i];
        const auto& i_required  = recipe_->ingredients[i];

        // No item, Wrong item or not enough of item
        if (i_possessed.item == nullptr || i_possessed.item->name != i_required.first ||
            i_possessed.count < i_required.second) {
            return false;
        }
    }

    return true;
}

void proto::AssemblyMachineData::CraftRemoveIngredients() {
    // Deduct ingredients
    for (size_t i = 0; i < recipe_->ingredients.size(); ++i) {
        ingredientInv[i].count -= recipe_->ingredients[i].second;

        if (ingredientInv[i].count == 0)
            ingredientInv[i].item = nullptr;
    }
}

void proto::AssemblyMachineData::CraftAddProduct() {
    // Add product
    assert(productInv[0].filter != nullptr);
    productInv[0].item = productInv[0].filter;
    productInv[0].count += recipe_->product.second;
}

// ======================================================================

SpriteFrameT proto::AssemblyMachine::OnRGetSpriteFrame(const UniqueDataBase& unique_data, GameTickT game_tick) const {
    const auto& machine_data = static_cast<const AssemblyMachineData&>(unique_data);

    if (!machine_data.deferralEntry.Valid())
        game_tick = 0;

    return AllOfSprite(*sprite, game_tick, 1. / 6);
}

bool proto::AssemblyMachine::OnRShowGui(const render::GuiRenderer& g_rendr, game::ChunkTileLayer* tile_layer) const {
    gui::AssemblyMachine({g_rendr, this, tile_layer->GetUniqueData()});
    return true;
}

// ======================================================================

bool proto::AssemblyMachine::TryBeginCrafting(game::Logic& logic, AssemblyMachineData& data) const {
    if (!data.CanBeginCrafting() || data.deferralEntry.Valid())
        return false;

    data.deferralEntry = logic.deferralTimer.RegisterFromTick(
        *this, &data, data.GetRecipe()->GetCraftingTime(1. / this->assemblySpeed));

    data.CraftRemoveIngredients();
    return true;
}


void proto::AssemblyMachine::OnDeferTimeElapsed(game::World& /*world*/,
                                                game::Logic& logic,
                                                UniqueDataBase* unique_data) const {
    auto* machine_data = static_cast<AssemblyMachineData*>(unique_data);

    machine_data->CraftAddProduct();

    machine_data->deferralEntry.Invalidate();
    TryBeginCrafting(logic, *machine_data);
}

void proto::AssemblyMachine::OnBuild(game::World& /*world*/,
                                     game::Logic& /*logic*/,
                                     const WorldCoord& /*coord*/,
                                     game::ChunkTileLayer& tile_layer,
                                     const Orientation /*orientation*/) const {
    tile_layer.MakeUniqueData<AssemblyMachineData>();
}

void proto::AssemblyMachine::OnRemove(game::World& /*world*/,
                                      game::Logic& logic,
                                      const WorldCoord& /*coord*/,
                                      game::ChunkTileLayer& tile_layer) const {
    auto& machine_data = *tile_layer.GetUniqueData<AssemblyMachineData>();

    logic.deferralTimer.RemoveDeferralEntry(machine_data.deferralEntry);
}
