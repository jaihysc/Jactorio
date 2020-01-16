#ifndef DATA_PROTOTYPE_ITEM_RECIPE_H
#define DATA_PROTOTYPE_ITEM_RECIPE_H

#include "data/prototype/prototype_base.h"
#include "data/prototype/item/recipe_category.h"

namespace jactorio::data
{
	class Recipe : public Prototype_base
	{
	public:
		Recipe() = default;

		PYTHON_PROP_REF(Recipe, Recipe_category, recipe_category);

		// Recipe props
		// TODO
	};
}

#endif // DATA_PROTOTYPE_ITEM_RECIPE_H
