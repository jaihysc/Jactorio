// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#include <gtest/gtest.h>

#include "data/data_manager.h"
#include "data/prototype/item/recipe.h"

namespace data::prototype
{
	TEST(recipe, get_item_recipe) {
		// Allows for fast lookup of item recipes instead of searching through an entire unordered_map

		using namespace jactorio::data;

		EXPECT_EQ(Recipe::get_item_recipe("non-existent-item"), nullptr);

		auto laptop_recipe = Recipe();
		laptop_recipe.set_product({"Laptop", 1});

		auto* recipe = Recipe::get_item_recipe("Laptop");
		EXPECT_EQ(recipe, &laptop_recipe);
	}

	/**
	 * Returns true if iname exists in array and matches count
	 */
	bool vector_get_val(std::vector<jactorio::data::recipe_item>& v, const std::string& iname, const uint16_t val) {
		for (auto& i : v) {
			if (i.first == iname) {
				if (i.second == val)
					return true;

				// Wrong value
				return false;
			}
		}
		// Not found
		return false;
	}

	TEST(recipe, recipe_get_total_raw) {
		namespace data_manager = jactorio::data::data_manager;

		auto guard = jactorio::core::Resource_guard(data_manager::clear_data);

		// Final item requires intermediate 1 (10), intermediate 2 (5)
		auto* recipe_final = new jactorio::data::Recipe();
		recipe_final->ingredients = {{"intermediate-1", 10}, {"intermediate-2", 5}};
		recipe_final->set_product({"final", 1});

		data_manager::data_raw_add("r-final", recipe_final);

		// Intermediate 1 requires raw 1 (12), raw 2 (3) -> produces 1
		auto* recipe_intermediate1 = new jactorio::data::Recipe();
		recipe_intermediate1->ingredients = {{"raw-1", 12}, {"raw-2", 3}};
		recipe_intermediate1->set_product({"intermediate-1", 1});

		data_manager::data_raw_add("r-intermediate1", recipe_intermediate1);

		// Intermediate 2 requires raw 1 (7) -> produces 2
		auto* recipe_intermediate2 = new jactorio::data::Recipe();
		recipe_intermediate2->ingredients = {{"raw-1", 7}};
		recipe_intermediate2->set_product({"intermediate-2", 2});

		data_manager::data_raw_add("r-intermediate2", recipe_intermediate2);


		// Get total raw
		auto v = jactorio::data::Recipe::recipe_get_total_raw("final");
		EXPECT_TRUE(vector_get_val(v, "raw-1", 141));
		EXPECT_TRUE(vector_get_val(v, "raw-2", 30));
	}

	TEST(recipe, post_load_validate) {
		const auto recipe = jactorio::data::Recipe();

		// Not specifying any ingredients or products should throw an data exception
		bool caught = false;
		try {
			recipe.post_load_validate();
		}
		catch (jactorio::data::Data_exception& e) {
			caught = true;
		}

		if (!caught) {
			FAIL();
		}
	}
}
