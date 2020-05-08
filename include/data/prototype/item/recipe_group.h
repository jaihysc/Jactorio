// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 01/20/2020

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_ITEM_RECIPE_GROUP_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_ITEM_RECIPE_GROUP_H
#pragma once

#include "data/prototype/item/recipe_category.h"
#include "data/prototype/prototype_base.h"
#include "data/prototype/sprite.h"

namespace jactorio::data
{
	/**
	 * Each Recipe_group gets its own tab within the recipe menu at the top, clicking one displays the recipes within
	 * that group
	 */
	class Recipe_group final : public Prototype_base
	{
	public:
		PROTOTYPE_CATEGORY(recipe_group);

		Recipe_group() = default;

		// Icon displayed in the menu
		Sprite* sprite = nullptr;

		Recipe_group* set_sprite(Sprite* (sprite)) {
			sprite->group.push_back(Sprite::SpriteGroup::gui);
			this->sprite = sprite;
			return this;
		}

		// Categories belonging to this Recipe_group
		PYTHON_PROP_REF(Recipe_group, std::vector<Recipe_category*>, recipe_categories);


		void post_load_validate() const override {
		}
	};
}

#endif //JACTORIO_INCLUDE_DATA_PROTOTYPE_ITEM_RECIPE_GROUP_H
