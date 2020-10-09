// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_PROTO_RECIPE_H
#define JACTORIO_INCLUDE_PROTO_RECIPE_H
#pragma once

#include <utility>

#include <vector>

#include "core/data_type.h"
#include "proto/detail/type.h"
#include "proto/framework/framework_base.h"

namespace jactorio::proto
{
    // Internal name, amount required
    using RecipeItem = std::pair<std::string, uint16_t>;

    ///
    /// Defines an in game recipe to craft items
    class Recipe final : public FrameworkBase
    {
    public:
        PROTOTYPE_CATEGORY(recipe);

        /// Seconds to complete recipe
        PYTHON_PROP_REF_I(ProtoFloatT, craftingTime, 1);

        PYTHON_PROP_REF(std::vector<RecipeItem>, ingredients);
        PYTHON_PROP_REF(RecipeItem, product);

        // ======================================================================

        void PostLoadValidate(const data::PrototypeManager& proto_manager) const override;

        ///
        /// Gets number of logic ticks necessary to craft recipe
        J_NODISCARD GameTickT GetCraftingTime(double multiplier = 1.) const;

        ///
        /// Looks up recipe for item of iname
        /// \returns nullptr if not found
        static const Recipe* GetItemRecipe(const data::PrototypeManager& data_manager, const std::string& iname);

        ///
        /// Returns raw materials for a recipe <br>
        /// Assumes all provided names are valid <br>
        /// A raw material is something which cannot be hand crafted
        static std::vector<RecipeItem> RecipeGetTotalRaw(const data::PrototypeManager& data_manager,
                                                         const std::string& iname);
    };
} // namespace jactorio::proto

#endif // JACTORIO_INCLUDE_PROTO_RECIPE_H
