#ifndef DATA_PROTOTYPE_ITEM_RECIPE_CATEGORY_H
#define DATA_PROTOTYPE_ITEM_RECIPE_CATEGORY_H

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
		Recipe_category() = default;

		// Recipes belonging to this category
		PYTHON_PROP_REF(Recipe_category, std::vector<Recipe*>, recipes);
	};
}

#endif // DATA_PROTOTYPE_ITEM_RECIPE_CATEGORY_H
