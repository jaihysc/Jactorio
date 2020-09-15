// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "data/prototype/recipe_group.h"

#include "data/prototype/sprite.h"

using namespace jactorio;

data::RecipeGroup* data::RecipeGroup::SetSprite(Sprite* sprite) {
    sprite->group.push_back(Sprite::SpriteGroup::gui);
    this->sprite = sprite;
    return this;
}
