// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "data/prototype_manager.h"
#include "proto/recipe.h"

namespace jactorio::proto
{
    class RecipeTest : public testing::Test
    {
    protected:
        data::PrototypeManager proto_;
    };

    TEST_F(RecipeTest, GetItemRecipe) {
        // Allows for fast lookup of item recipes instead of searching through an entire unordered_map
        EXPECT_EQ(Recipe::GetItemRecipe(proto_, "non-existent-item"), nullptr);

        auto& laptop_recipe   = proto_.Make<Recipe>();
        laptop_recipe.product = {"Laptop", 1};

        const auto* recipe = Recipe::GetItemRecipe(proto_, "Laptop");
        EXPECT_EQ(recipe, &laptop_recipe);
    }

    ///
    /// Returns true if iname exists in array and matches count
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
        auto& recipe_final = proto_.Make<Recipe>("r-final");

        recipe_final.ingredients = {{"intermediate-1", 10}, {"intermediate-2", 5}};
        recipe_final.product     = {"final", 1};

        // Intermediate 1 requires raw 1 (12), raw 2 (3) -> produces 1
        auto& recipe_intermediate1       = proto_.Make<Recipe>("r-intermediate1");
        recipe_intermediate1.ingredients = {{"raw-1", 12}, {"raw-2", 3}};
        recipe_intermediate1.product     = {"intermediate-1", 1};


        // Intermediate 2 requires raw 1 (7) -> produces 2
        auto& recipe_intermediate2       = proto_.Make<Recipe>("r-intermediate2");
        recipe_intermediate2.ingredients = {{"raw-1", 7}};
        recipe_intermediate2.product     = {"intermediate-2", 2};


        // Get total raw
        auto v = Recipe::RecipeGetTotalRaw(proto_, "final");
        EXPECT_TRUE(VectorGetVal(v, "raw-1", 141));
        EXPECT_TRUE(VectorGetVal(v, "raw-2", 30));
    }

    TEST_F(RecipeTest, GetCraftingTime) {
        Recipe recipe;
        recipe.craftingTime = 1.5;

        EXPECT_EQ(recipe.GetCraftingTime(), 90);

        EXPECT_EQ(recipe.GetCraftingTime(1.f / 2), 45);
    }

    TEST_F(RecipeTest, PostLoadValidate) {
        const Recipe recipe;

        // Not specifying any ingredients or products should throw an data exception
        bool caught = false;
        try {
            recipe.PostLoadValidate(proto_);
        }
        catch (ProtoError&) {
            caught = true;
        }

        if (!caught) {
            FAIL();
        }
    }
} // namespace jactorio::proto
