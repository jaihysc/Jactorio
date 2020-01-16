#ifndef DATA_PROTOTYPE_ITEM_RECIPE_CATEGORY_H
#define DATA_PROTOTYPE_ITEM_RECIPE_CATEGORY_H

#include "data/prototype/prototype_base.h"

namespace jactorio::data
{
	/**
	 * A category for recipes, the localized_name is printed
	 */
	class Recipe_category final : public Prototype_base
	{
	public:
		Recipe_category() = default;
	};
}

#endif // DATA_PROTOTYPE_ITEM_RECIPE_CATEGORY_H
