// 
// recipe_category.h
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 01/20/2020
// Last modified: 04/07/2020
// 

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_ITEM_RECIPE_CATEGORY_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_ITEM_RECIPE_CATEGORY_H
#pragma once

#include "data/prototype/prototype_base.h"
#include "data/prototype/item/recipe.h"

namespace jactorio::data
{
	/**
	 * A category to sort recipes within a recipe group
	 */
	class Recipe_category final : public Prototype_base
	{
	public:
		PROTOTYPE_CATEGORY(recipe_category);

		Recipe_category() = default;

		// Recipes belonging to this category
		PYTHON_PROP_REF(Recipe_category, std::vector<Recipe*>, recipes);

		void post_load_validate() const override {
		}
	};
}

#endif //JACTORIO_INCLUDE_DATA_PROTOTYPE_ITEM_RECIPE_CATEGORY_H
