#ifndef DATA_PROTOTYPE_ITEM_RECIPE_GROUP_H
#define DATA_PROTOTYPE_ITEM_RECIPE_GROUP_H

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
		Recipe_group() = default;

		// Icon displayed in the menu
		Sprite* sprite = nullptr;

		Recipe_group* set_sprite(Sprite* (sprite)) {
			sprite->group = Sprite::sprite_group::gui;
			this->sprite = sprite;
			return this;
		};
	};
}

#endif // DATA_PROTOTYPE_ITEM_RECIPE_GROUP_H
