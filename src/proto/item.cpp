// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "proto/item.h"

#include "proto/sprite.h"

using namespace jactorio;

void proto::Item::ValidatedPostLoad() {
    sprite->DefaultSpriteGroup({Sprite::SpriteGroup::terrain, Sprite::SpriteGroup::gui});
}
