#ifndef DATA_PROTOTYPE_ITEM_RECIPE_CATEGORY_H
#define DATA_PROTOTYPE_ITEM_RECIPE_CATEGORY_H

#include "data/prototype/prototype_base.h"
#include "data/prototype/item/recipe_group.h"

namespace jactorio::data
{
	/**
	 * A category to sort recipes within a recipe group
	 */
	class Recipe_category final : public Prototype_base
	{
	public:
		Recipe_category()
			: recipe_group(nullptr) {
		}

		// Group which this category falls under
		PYTHON_PROP(Recipe_category, Recipe_group*, recipe_group);
	};
}

#endif // DATA_PROTOTYPE_ITEM_RECIPE_CATEGORY_H
