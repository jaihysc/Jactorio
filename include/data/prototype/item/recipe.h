#ifndef DATA_PROTOTYPE_ITEM_RECIPE_H
#define DATA_PROTOTYPE_ITEM_RECIPE_H

#include <utility>

#include "data/prototype/prototype_base.h"

namespace jactorio::data
{
	/**
	 * Defines an in game recipe to craft items
	 */
	class Recipe : public Prototype_base
	{
	public:
		Recipe() = default;

		// std::string is internal name

		// Internal name, amount required
		std::vector<std::pair<std::string, uint16_t>> ingredients;

		Recipe* set_ingredients(const std::vector<std::pair<std::string, uint16_t>>& ingredients) {
			this->ingredients = ingredients;
			return this;
		};

		
		PYTHON_PROP_REF(Recipe, std::string, product);
	};
}

#endif // DATA_PROTOTYPE_ITEM_RECIPE_H
