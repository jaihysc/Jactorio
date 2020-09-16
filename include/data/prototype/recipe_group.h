// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_RECIPE_GROUP_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_RECIPE_GROUP_H
#pragma once

#include "data/prototype/framework/framework_base.h"

namespace jactorio::data
{
    class RecipeCategory;
    class Sprite;

    ///
    /// Each Recipe_group gets its own tab within the recipe menu at the top, clicking one displays the recipes
    /// within that group
    class RecipeGroup final : public FrameworkBase
    {
    public:
        PROTOTYPE_CATEGORY(recipe_group);

        RecipeGroup() = default;


        // Icon displayed in the menu
        Sprite* sprite = nullptr;

        RecipeGroup* SetSprite(Sprite* sprite);


        // Categories belonging to this Recipe_group
        PYTHON_PROP_REF(std::vector<RecipeCategory*>, recipeCategories);


        void PostLoadValidate(const PrototypeManager& /*proto_manager*/) const override {}
    };
} // namespace jactorio::data

#endif // JACTORIO_INCLUDE_DATA_PROTOTYPE_RECIPE_GROUP_H
