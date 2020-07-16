// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_ITEM_RECIPE_GROUP_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_ITEM_RECIPE_GROUP_H
#pragma once

#include "data/prototype/prototype_base.h"
#include "data/prototype/sprite.h"
#include "data/prototype/item/recipe_category.h"

namespace jactorio::data
{
	///
	/// \brief Each Recipe_group gets its own tab within the recipe menu at the top, clicking one displays the recipes within
	/// that group
	class RecipeGroup final : public PrototypeBase
	{
	public:
		PROTOTYPE_CATEGORY(recipe_group);

		RecipeGroup() = default;

		// Icon displayed in the menu
		Sprite* sprite = nullptr;

		RecipeGroup* SetSprite(Sprite* (sprite)) {
			sprite->group.push_back(Sprite::SpriteGroup::gui);
			this->sprite = sprite;
			return this;
		}

		// Categories belonging to this Recipe_group
		PYTHON_PROP_REF(RecipeGroup, std::vector<RecipeCategory*>, recipeCategories);


		void PostLoadValidate(const PrototypeManager&) const override {
		}
	};
}

#endif //JACTORIO_INCLUDE_DATA_PROTOTYPE_ITEM_RECIPE_GROUP_H
