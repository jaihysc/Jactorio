// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "data/prototype_manager.h"
#include "data/prototype/item/recipe.h"

namespace jactorio::data
{
	class RecipeTest : public testing::Test
	{
	protected:
		PrototypeManager dataManager_{};
	};

	TEST_F(RecipeTest, GetItemRecipe) {
		// Allows for fast lookup of item recipes instead of searching through an entire unordered_map
		EXPECT_EQ(jactorio::data::Recipe::GetItemRecipe(dataManager_, "non-existent-item"), nullptr);

		auto* laptop_recipe    = new Recipe();
		laptop_recipe->product = {"Laptop", 1};
		dataManager_.DataRawAdd("", laptop_recipe, false);

		const auto* recipe = Recipe::GetItemRecipe(dataManager_, "Laptop");
		EXPECT_EQ(recipe, laptop_recipe);
	}

	///
	/// \brief Returns true if iname exists in array and matches count
	bool VectorGetVal(std::vector<RecipeItem>& vector, const std::string& iname, const uint16_t val) {
		for (auto& i : vector) {
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

	TEST_F(RecipeTest, RecipeGetTotalRaw) {
		// Final item requires intermediate 1 (10), intermediate 2 (5)
		auto* recipe_final        = new Recipe();
		recipe_final->ingredients = {{"intermediate-1", 10}, {"intermediate-2", 5}};
		recipe_final->product     = {"final", 1};

		dataManager_.DataRawAdd("r-final", recipe_final);

		// Intermediate 1 requires raw 1 (12), raw 2 (3) -> produces 1
		auto* recipe_intermediate1        = new Recipe();
		recipe_intermediate1->ingredients = {{"raw-1", 12}, {"raw-2", 3}};
		recipe_intermediate1->product     = {"intermediate-1", 1};

		dataManager_.DataRawAdd("r-intermediate1", recipe_intermediate1);

		// Intermediate 2 requires raw 1 (7) -> produces 2
		auto* recipe_intermediate2        = new Recipe();
		recipe_intermediate2->ingredients = {{"raw-1", 7}};
		recipe_intermediate2->product     = {"intermediate-2", 2};

		dataManager_.DataRawAdd("r-intermediate2", recipe_intermediate2);


		// Get total raw
		auto v = Recipe::RecipeGetTotalRaw(dataManager_, "final");
		EXPECT_TRUE(VectorGetVal(v, "raw-1", 141));
		EXPECT_TRUE(VectorGetVal(v, "raw-2", 30));
	}

	TEST_F(RecipeTest, GetCraftingTime) {
		Recipe recipe{};
		recipe.craftingTime = 1.5;

		EXPECT_EQ(recipe.GetCraftingTime(), 90);

		EXPECT_EQ(recipe.GetCraftingTime(1.f/2), 45);
	}

	TEST_F(RecipeTest, PostLoadValidate) {
		const auto recipe = Recipe();

		// Not specifying any ingredients or products should throw an data exception
		bool caught = false;
		try {
			recipe.PostLoadValidate(dataManager_);
		}
		catch (DataException&) {
			caught = true;
		}

		if (!caught) {
			FAIL();
		}
	}
}
