// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "proto/recipe_group.h"

#include "proto/sprite.h"

using namespace jactorio;

proto::RecipeGroup* proto::RecipeGroup::SetSprite(Sprite* sprite) {
    this->sprite = sprite;
    return this;
}

void proto::RecipeGroup::PostLoadValidate(const data::PrototypeManager&) const {
    J_PROTO_ASSERT(sprite != nullptr, "RecipeGroup sprite was not specified");
    J_PROTO_ASSERT(sprite->group == Sprite::SpriteGroup::gui, "RecipeGroup sprite must be in group GUI");
}
