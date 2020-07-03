// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 01/20/2020

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_ITEM_RECIPE_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_ITEM_RECIPE_H
#pragma once

#include <utility>

#include <unordered_map>
#include <vector>

#include "data/data_manager.h"
#include "data/prototype/prototype_base.h"

namespace jactorio::data
{
	// Internal name, amount required
	using RecipeItem = std::pair<std::string, uint16_t>;

	///
	/// \brief Defines an in game recipe to craft items
	class Recipe final : public PrototypeBase
	{
	public:
		PROTOTYPE_CATEGORY(recipe);

		/// \brief Seconds to complete recipe
		PYTHON_PROP_REF_I(Recipe, float, craftingTime, 1);

		PYTHON_PROP_REF(Recipe, std::vector<RecipeItem>, ingredients);
		PYTHON_PROP_REF(Recipe, RecipeItem, product);

		// ======================================================================
		
		void PostLoadValidate(const DataManager&) const override;

		///
		/// \brief Looks up recipe for item of iname
		/// \returns nullptr if not found
		static const Recipe* GetItemRecipe(const DataManager& data_manager, const std::string& iname);

		///
		/// \brief Returns raw materials for a recipe <br>
		/// Assumes all provided names are valid <br>
		/// A raw material is something which cannot be hand crafted
		static std::vector<RecipeItem> RecipeGetTotalRaw(const DataManager& data_manager, const std::string& iname);
	};
}

#endif //JACTORIO_INCLUDE_DATA_PROTOTYPE_ITEM_RECIPE_H
