// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "proto/recipe_group.h"

#include "proto/sprite.h"

using namespace jactorio;

proto::RecipeGroup* proto::RecipeGroup::SetSprite(Sprite* sprite) {
    sprite->group.push_back(Sprite::SpriteGroup::gui);
    this->sprite = sprite;
    return this;
}
