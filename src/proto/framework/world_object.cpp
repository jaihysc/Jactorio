// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_PROTO_FRAMEWORK_WORLD_OBJECT_CPP
#define JACTORIO_INCLUDE_PROTO_FRAMEWORK_WORLD_OBJECT_CPP
#pragma once

#include "proto/framework/world_object.h"

#include "proto/sprite.h"

using namespace jactorio;

SpriteTexCoordIndexT proto::FWorldObject::OnGetTexCoordId(const game::World& /*world*/,
                                                          const WorldCoord& /*coord*/,
                                                          Orientation /*orientation*/) const {
    // TODO rotations
    return sprite->texCoordId;
}

#endif // JACTORIO_INCLUDE_PROTO_FRAMEWORK_WORLD_OBJECT_CPP
