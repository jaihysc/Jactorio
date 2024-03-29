// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "proto/framework/world_object.h"

#include "proto/sprite.h"

using namespace jactorio;

SpriteTexCoordIndexT proto::FWorldObject::OnGetTexCoordId(const game::World& /*world*/,
                                                          const WorldCoord& /*coord*/,
                                                          const Orientation orientation) const {
    assert(sprite != nullptr);

    // If one of the other direction's sprites is given, assume they want to map orientation to sprite
    if (spriteE != nullptr) {
        assert(spriteS != nullptr);
        assert(spriteW != nullptr);

        switch (orientation) {
        case Orientation::up:
            return sprite->texCoordId;
        case Orientation::right:
            return spriteE->texCoordId;
        case Orientation::down:
            return spriteS->texCoordId;
        case Orientation::left:
            return spriteW->texCoordId;

        default:
            assert(false);
            return 0;
        }
    }
    else {
        return sprite->texCoordId;
    }
}

void proto::FWorldObject::PostLoadValidate(const data::PrototypeManager& /*proto*/) const {
    J_PROTO_ASSERT(sprite != nullptr, "Sprite was not specified");
    J_PROTO_ASSERT(sprite->group == Sprite::SpriteGroup::terrain, "Sprite must be in group TERRAIN");

    if (spriteE != nullptr || spriteS != nullptr || spriteW != nullptr) {
        J_PROTO_ASSERT(spriteE != nullptr, "spriteE was not specified");
        J_PROTO_ASSERT(spriteS != nullptr, "spriteS was not specified");
        J_PROTO_ASSERT(spriteW != nullptr, "spriteW was not specified");
        J_PROTO_ASSERT(spriteE->group == Sprite::SpriteGroup::terrain, "SpriteE must be in group TERRAIN");
        J_PROTO_ASSERT(spriteS->group == Sprite::SpriteGroup::terrain, "SpriteS must be in group TERRAIN");
        J_PROTO_ASSERT(spriteW->group == Sprite::SpriteGroup::terrain, "SpriteW must be in group TERRAIN");
    }
}
