// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "proto/splitter.h"

using namespace jactorio;

void proto::Splitter::PostLoadValidate(const data::PrototypeManager& /*proto_manager*/) const {
    J_PROTO_ASSERT(this->tileWidth == 2, "Tile width must be 2");
    J_PROTO_ASSERT(this->tileHeight == 1, "Tile height must be 1");
}
