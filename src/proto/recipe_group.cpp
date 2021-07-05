// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "proto/recipe_group.h"

using namespace jactorio;

proto::RecipeGroup* proto::RecipeGroup::SetSprite(Sprite* sprite) {
    this->sprite = sprite;
    return this;
}
