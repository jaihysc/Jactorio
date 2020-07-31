// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "data/prototype/item/recipe.h"

#include <mutex>

#include "core/data_type.h"

#include "data/prototype_manager.h"
#include "data/prototype/item/item.h"

///
/// \brief Recursively resolves raw materials
void ResolveRawRecipe(const jactorio::data::PrototypeManager& data_manager,
                      std::unordered_map<std::string, uint16_t>& materials_raw,
                      const jactorio::data::Recipe* recipe, const uint16_t amount) {
	using namespace jactorio;

	for (const auto& recipe_ingredient : recipe->ingredients) {
		const auto* ingredient_recipe = data::Recipe::GetItemRecipe(data_manager, recipe_ingredient.first);

		// No recipe means this is a raw material
		if (ingredient_recipe == nullptr) {
			// Add raw material to map
			materials_raw[recipe_ingredient.first] += recipe_ingredient.second *
				amount;  // ingredient count * #product needed
			continue;
		}

		// To calculate amount required, find the next highest multiple of the ingredient recipe
		// e.g: 5 required, recipe in batches of 2 = 6
		unsigned int x = recipe_ingredient.second * amount, y = ingredient_recipe->product.second, q;
		q              = (x + y - 1) / y;
		ResolveRawRecipe(data_manager, materials_raw, ingredient_recipe, q);
	}

}

const jactorio::data::Recipe* jactorio::data::Recipe::GetItemRecipe(const PrototypeManager& data_manager,
                                                                    const std::string& iname) {
	const auto recipes = data_manager.DataRawGetAll<const Recipe>(DataCategory::recipe);

	for (const auto& recipe : recipes) {
		if (recipe->product.first == iname)
			return recipe;
	}

	return nullptr;
}

std::vector<jactorio::data::RecipeItem> jactorio::data::Recipe::RecipeGetTotalRaw(const PrototypeManager& data_manager,
                                                                                  const std::string& iname) {
	// Key is ptr instead of std::string for some added speed
	std::unordered_map<std::string, uint16_t> map_raw;

	const auto* recipe = GetItemRecipe(data_manager, iname);
	ResolveRawRecipe(data_manager, map_raw, recipe, 1);

	// Transform map into vector
	std::vector<RecipeItem> v;
	v.reserve(map_raw.size());
	for (auto& item : map_raw) {
		v.emplace_back(item.first, item.second);
	}
	return v;
}

void jactorio::data::Recipe::PostLoadValidate(const PrototypeManager& data_manager) const {
	J_DATA_ASSERT(!ingredients.empty(), "No ingredients specified for recipe");
	for (const auto& ingredient : ingredients) {
		J_DATA_ASSERT(!ingredient.first.empty(), "Empty ingredient internal name specifier");

		const auto* ingredient_item = data_manager.DataRawGet<Item>(ingredient.first);
		J_DATA_ASSERT_F(ingredient_item, "Ingredient %s does not exist", ingredient.first.c_str());

		J_DATA_ASSERT(ingredient.second > 0, "Ingredient required amount minimum is 1");
		J_DATA_ASSERT_F(ingredient.second <= ingredient_item->stackSize,
		                "Ingredient required amount %d exceeds max stack size of ingredient %d",
						ingredient.second, ingredient_item->stackSize);
	}

	J_DATA_ASSERT(!product.first.empty(), "No product specified for recipe");

	const auto* item_product = data_manager.DataRawGet<Item>(product.first);
	J_DATA_ASSERT_F(item_product != nullptr, "Product %s does not exist", product.first.c_str());

	J_DATA_ASSERT(product.second > 0, "Product yield amount minimum is 1");
	J_DATA_ASSERT_F(product.second <= item_product->stackSize,
	                "Product yield %d may not exceed product stack size %d", product.second, item_product->stackSize);
}

jactorio::GameTickT jactorio::data::Recipe::GetCraftingTime(const double multiplier) const {
	return craftingTime * multiplier * kGameHertz;
}
