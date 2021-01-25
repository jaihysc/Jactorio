// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_WORLD_LOGIC_GROUP_H
#define JACTORIO_INCLUDE_GAME_WORLD_LOGIC_GROUP_H
#pragma once

namespace jactorio::game
{
    enum class LogicGroup
    {
        conveyor = 0,
        inserter,
        count_
    };

    static constexpr auto kLogicGroupCount = static_cast<int>(LogicGroup::count_);
} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_WORLD_LOGIC_GROUP_H
