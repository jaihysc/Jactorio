// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 01/20/2020

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_ITEM_RECIPE_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_ITEM_RECIPE_H
#pragma once

#include <utility>

#include <unordered_map>
#include <vector>
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

		Recipe() = default;


		/// \brief Seconds to complete recipe
		PYTHON_PROP_REF_I(Recipe, float, craftingTime, 1);


		PYTHON_PROP_REF(Recipe, std::vector<RecipeItem>, ingredients);

		// Product
		J_NODISCARD RecipeItem GetProduct() const { return this->product_; }

		Recipe* SetProduct(const RecipeItem& (product)) {
			// Save recipe in lookup
			itemRecipes_[product.first] = this;

			this->product_ = product;
			return this;
		}


		void PostLoadValidate() const override;

	private:
		static std::unordered_map<std::string, Recipe*> itemRecipes_;
		RecipeItem product_;

	public:
		///
		/// \brief Looks up recipe for item of iname
		/// \returns nullptr if not found
		static Recipe* GetItemRecipe(const std::string& iname);

		///
		/// \brief Returns raw materials for a recipe <br>
		/// Assumes all provided names are valid <br>
		/// A raw material is something which cannot be hand crafted
		static std::vector<RecipeItem> RecipeGetTotalRaw(const std::string& iname);
	};
}

#endif //JACTORIO_INCLUDE_DATA_PROTOTYPE_ITEM_RECIPE_H
