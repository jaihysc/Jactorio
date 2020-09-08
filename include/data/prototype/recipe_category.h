// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_ITEM_RECIPE_CATEGORY_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_ITEM_RECIPE_CATEGORY_H
#pragma once

#include "data/prototype/framework/framework_base.h"
#include "data/prototype/recipe.h"

namespace jactorio::data
{
    ///
    /// A category to sort recipes within a recipe group
    class RecipeCategory final : public FrameworkBase
    {
    public:
        PROTOTYPE_CATEGORY(recipe_category);

        RecipeCategory() = default;

        // Recipes belonging to this category
        PYTHON_PROP_REF(std::vector<Recipe*>, recipes);

        void PostLoadValidate(const PrototypeManager&) const override {}
    };
} // namespace jactorio::data

#endif // JACTORIO_INCLUDE_DATA_PROTOTYPE_ITEM_RECIPE_CATEGORY_H
