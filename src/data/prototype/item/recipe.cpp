#include "data/prototype/item/recipe.h"

#include <mutex>

std::unordered_map<std::string, jactorio::data::Recipe*> jactorio::data::Recipe::item_recipes_{};
std::mutex mutex;

jactorio::data::Recipe* jactorio::data::Recipe::get_item_recipe(const std::string& iname) {
	auto lk = std::lock_guard<std::mutex>(mutex);

	// Does not exist
	if (item_recipes_.find(iname) == item_recipes_.end())
		return nullptr;

	return item_recipes_[iname];
}

/**
 * Recursively resolves raw materials
 */
void resolve_raw_recipe(std::unordered_map<std::string, uint16_t>& materials_raw,
                        jactorio::data::Recipe* recipe, const uint16_t amount) {
	using namespace jactorio;

	for (auto& recipe_ingredient : recipe->ingredients) {
		auto* ingredient_recipe = data::Recipe::get_item_recipe(recipe_ingredient.first);

		// No recipe means this is a raw material
		if (ingredient_recipe == nullptr) {
			// Add raw material to map
			materials_raw[recipe_ingredient.first] += recipe_ingredient.second *
				amount;  // ingredient count * #product needed
			continue;
		}

		// To calculate amount required, find the next highest multiple of the ingredient recipe
		// e.g: 5 required, recipe in batches of 2 = 6
		unsigned int x = recipe_ingredient.second * amount, y = ingredient_recipe->get_product().second, q;
		q = (x + y - 1) / y;
		resolve_raw_recipe(materials_raw, ingredient_recipe, q);
	}

}

std::vector<jactorio::data::recipe_item> jactorio::data::Recipe::recipe_get_total_raw(const std::string& iname) {
	// Key is ptr instead of std::string for some added speed
	std::unordered_map<std::string, uint16_t> map_raw;

	const auto recipe = get_item_recipe(iname);
	resolve_raw_recipe(map_raw, recipe, 1);

	// Transform map into vector
	std::vector<recipe_item> v;
	v.reserve(map_raw.size());
	for (auto& item : map_raw) {
		v.emplace_back(item.first, item.second);
	}
	return v;
}

void jactorio::data::Recipe::post_load_validate() const {
	J_DATA_ASSERT(!ingredients.empty(), "No ingredients specified for recipe")
	for (auto& ingredient : ingredients) {
		J_DATA_ASSERT(!ingredient.first.empty(), "Empty ingredient internal name specifier");
		J_DATA_ASSERT(ingredient.second > 0, "Ingredient required amount minimum is 1");
	}
	
	J_DATA_ASSERT(!product_.first.empty(), "No product specified for recipe");
	J_DATA_ASSERT(product_.second > 0, "Product yield amount minimum is 1");
}
