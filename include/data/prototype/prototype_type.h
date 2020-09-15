// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_PROTOTYPE_TYPE_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_PROTOTYPE_TYPE_H
#pragma once

#include "core/coordinate_tuple.h"
#include "core/data_type.h"
#include "data/prototype/type.h"

namespace jactorio::data
{
    ///
    /// Represents an output location for an entity on 4 orientation
    struct Tile4Way
    {
        Tile4Way() = default;

        Tile4Way(const WorldCoord& up, const WorldCoord& right, const WorldCoord& down, const WorldCoord& left)
            : up(up), right(right), down(down), left(left) {}

        // For Python API
        explicit Tile4Way(std::tuple<std::pair<WorldCoordAxis, WorldCoordAxis>,
                                     std::pair<WorldCoordAxis, WorldCoordAxis>,
                                     std::pair<WorldCoordAxis, WorldCoordAxis>,
                                     std::pair<WorldCoordAxis, WorldCoordAxis>> t)
            : up({std::get<0>(t).first, std::get<0>(t).second}),
              right({std::get<1>(t).first, std::get<1>(t).second}),
              down({std::get<2>(t).first, std::get<2>(t).second}),
              left({std::get<3>(t).first, std::get<3>(t).second}) {}


        WorldCoord up;
        WorldCoord right;
        WorldCoord down;
        WorldCoord left;

        J_NODISCARD const WorldCoord& Get(const Orientation orientation) const {
            switch (orientation) {

            case Orientation::up:
                return up;
            case Orientation::right:
                return right;
            case Orientation::down:
                return down;
            case Orientation::left:
                return left;

            default:
                assert(false);
                return up;
            }
        }
    };
} // namespace jactorio::data

#endif // JACTORIO_INCLUDE_DATA_PROTOTYPE_PROTOTYPE_TYPE_H
