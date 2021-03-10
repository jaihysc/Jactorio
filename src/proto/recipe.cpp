// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "proto/recipe.h"

#include <mutex>

#include "core/convert.h"
#include "core/data_type.h"
#include "data/prototype_manager.h"
#include "proto/item.h"

using namespace jactorio;

/// Recursively resolves raw materials
void ResolveRawRecipe(const data::PrototypeManager& proto,
                      std::unordered_map<std::string, uint16_t>& materials_raw,
                      const proto::Recipe* recipe,
                      const uint16_t craft_amount) {
    using namespace jactorio;

    for (const auto& [ing_name, ing_needed_amount] : recipe->ingredients) {
        const auto* ing_recipe = proto::Recipe::GetItemRecipe(proto, ing_name);

        // No recipe means this is a raw material
        if (ing_recipe == nullptr) {
            materials_raw[ing_name] += ing_needed_amount * craft_amount;
            continue;
        }

        const auto craft_needed_amount = ing_needed_amount * craft_amount;
        const auto craft_amount_per    = ing_recipe->product.second;

        // To calculate amount required, find the next highest multiple of the ingredient recipe
        // e.g: 5 required, recipe in batches of 2 = 3 crafts required

        const auto batches_required = (craft_needed_amount + craft_amount_per - 1) / craft_amount_per;
        ResolveRawRecipe(proto, materials_raw, ing_recipe, batches_required);
    }
}

const proto::Recipe* proto::Recipe::GetItemRecipe(const data::PrototypeManager& proto, const std::string& iname) {
    const auto recipes = proto.GetAll<const Recipe>();

    for (const auto& recipe : recipes) {
        if (recipe->product.first == iname)
            return recipe;
    }

    return nullptr;
}

std::vector<proto::RecipeItem> proto::Recipe::RecipeGetTotalRaw(const data::PrototypeManager& proto,
                                                                const std::string& iname) {
    std::unordered_map<std::string, uint16_t> map_raw;

    const auto* recipe = GetItemRecipe(proto, iname);
    ResolveRawRecipe(proto, map_raw, recipe, 1);

    // Transform map into vector
    std::vector<RecipeItem> v;
    v.reserve(map_raw.size());
    for (auto& item : map_raw) {
        v.emplace_back(item);
    }
    return v;
}

void proto::Recipe::PostLoadValidate(const data::PrototypeManager& proto) const {
    J_PROTO_ASSERT(!ingredients.empty(), "No ingredients specified for recipe");
    for (const auto& [ing_name, ing_req_amount] : ingredients) {
        J_PROTO_ASSERT(!ing_name.empty(), "Empty ingredient internal name specifier");

        const auto* ingredient_item = proto.Get<Item>(ing_name);
        assert(ingredient_item != nullptr);

        J_PROTO_ASSERT_F(ingredient_item, "Ingredient %s does not exist", ing_name.c_str());

        J_PROTO_ASSERT(ing_req_amount > 0, "Ingredient required amount minimum is 1");
        J_PROTO_ASSERT_F(ing_req_amount <= ingredient_item->stackSize,
                         "Ingredient required amount %d exceeds max stack size of ingredient %d",
                         ing_req_amount,
                         ingredient_item->stackSize);
    }

    J_PROTO_ASSERT(!product.first.empty(), "No product specified for recipe");

    const auto* item_product = proto.Get<Item>(product.first);
    assert(item_product != nullptr);

    J_PROTO_ASSERT_F(item_product != nullptr, "Product %s does not exist", product.first.c_str());

    J_PROTO_ASSERT(product.second > 0, "Product yield amount minimum is 1");
    J_PROTO_ASSERT_F(product.second <= item_product->stackSize,
                     "Product yield %d may not exceed product stack size %d",
                     product.second,
                     item_product->stackSize);
}

GameTickT proto::Recipe::GetCraftingTime(const double multiplier) const {
    return LossyCast<GameTickT>(craftingTime * multiplier * kGameHertz);
}
