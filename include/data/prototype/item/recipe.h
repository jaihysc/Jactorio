// 
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
	using recipe_item = std::pair<std::string, uint16_t>;

	/**
	 * Defines an in game recipe to craft items
	 */
	class Recipe : public Prototype_base
	{
	public:
		PROTOTYPE_CATEGORY(recipe);

		Recipe() = default;


		/**
		 * Seconds to complete recipe
		 */
		PYTHON_PROP_REF_I(Recipe, float, crafting_time, 1);


		PYTHON_PROP_REF(Recipe, std::vector<recipe_item>, ingredients);

		// Product
		J_NODISCARD recipe_item get_product() const { return this->product_; }

		Recipe* set_product(const recipe_item& (product)) {
			// Save recipe in lookup
			item_recipes_[product.first] = this;

			this->product_ = product;
			return this;
		}


		void post_load_validate() const override;

	private:
		static std::unordered_map<std::string, Recipe*> item_recipes_;
		recipe_item product_;

	public:
		/**
		 * Looks up recipe for item of iname
		 * @returns nullptr if not found
		 */
		static Recipe* get_item_recipe(const std::string& iname);

		/**
		 * Returns raw materials for a recipe <br>
		 * Assumes all provided names are valid <br>
		 * A raw material is something which cannot be hand crafted
		 */
		static std::vector<recipe_item> recipe_get_total_raw(const std::string& iname);
	};
}

#endif //JACTORIO_INCLUDE_DATA_PROTOTYPE_ITEM_RECIPE_H
