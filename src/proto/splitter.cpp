// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "proto/splitter.h"

#include "proto/sprite.h"

using namespace jactorio;

void proto::Splitter::PostLoad() {
    // Convert floating point speed to fixed precision decimal speed
    speed = LineDistT(speedFloat);
}

void proto::Splitter::PostLoadValidate(const data::PrototypeManager& proto_manager) const {
    Conveyor::PostLoadValidate(proto_manager);

    J_PROTO_ASSERT(this->GetWidth(Orientation::up) == 2, "Tile width must be 2");
    J_PROTO_ASSERT(this->GetHeight(Orientation::up) == 1, "Tile height must be 1");
}

void proto::Splitter::ValidatedPostLoad() {
    sprite->DefaultSpriteGroup({Sprite::SpriteGroup::terrain});
    spriteE->DefaultSpriteGroup({Sprite::SpriteGroup::terrain});
    spriteS->DefaultSpriteGroup({Sprite::SpriteGroup::terrain});
    spriteW->DefaultSpriteGroup({Sprite::SpriteGroup::terrain});
}
