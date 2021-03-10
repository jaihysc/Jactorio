// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_LOGIC_INSERTER_CONTROLLER_H
#define JACTORIO_INCLUDE_GAME_LOGIC_INSERTER_CONTROLLER_H
#pragma once

#include "core/data_type.h"
#include "core/math.h"

namespace jactorio::game
{
    class World;
    class Logic;

    // Inserter behavior
    //
    // Conveyors:
    //     Take from near side, place on near side
    //     Take from anywhere within 1 tile on the conveyor
    //     Put in center of output conveyor

    constexpr auto kInserterCenterOffset = 0.5;
    /// Distance from inserter arm radius to the end of the last tile
    constexpr auto kInserterArmTileGap = 0.3;

    constexpr auto kMinInserterDegree = 0;
    constexpr auto kMaxInserterDegree = 180;

    /// Gets the tile distance of the inserter arm to its resting position at 0 or 180 degrees
    ///
    ///   / |
    ///  /  | x
    /// /   |
    /// ----------- 0 or 180
    /// \param degree Current degree of inserter arm 0 <= degree <= 180
    /// \param target_distance Tiles to the location which the inserter picks up / drops items
    double GetInserterArmOffset(TIntDegree degree, unsigned target_distance);

    /// Gets the tile length of the inserter arm
    ///
    ///   / |
    /// x/  |
    /// /   |
    /// ----------- 0 or 180
    /// \param degree Current degree of inserter arm 0 <= degree <= 180
    /// \param target_distance Tiles to the location which the inserter picks up / drops items
    double GetInserterArmLength(TIntDegree degree, unsigned target_distance);


    /// Updates inserter logic for a logic chunk
    void InserterLogicUpdate(World& world, Logic& logic);
} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_LOGIC_INSERTER_CONTROLLER_H
