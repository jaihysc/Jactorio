// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_PLAYER_PLAYER_ACTION_H
#define JACTORIO_INCLUDE_GAME_PLAYER_PLAYER_ACTION_H
#pragma once

#include <functional>

#include "game/game_data.h"

namespace jactorio::game
{
    struct GameDataGlobal;

    // Associates enum key (action) with method (executor)
#define J_PLAYER_ACTIONS                                \
    J_CREATE_ACTION(player_move_up, PlayerMoveUp)       \
    J_CREATE_ACTION(player_move_right, PlayerMoveRight) \
    J_CREATE_ACTION(player_move_down, PlayerMoveDown)   \
    J_CREATE_ACTION(player_move_left, PlayerMoveLeft)   \
    J_CREATE_ACTION(test, ActionTest)

    struct PlayerAction
    {
        using Executor = std::function<void(GameDataGlobal& data_global)>;

#define J_CREATE_ACTION(enum_name__, executor__) enum_name__,
        enum class Type
        {
            J_PLAYER_ACTIONS

                count_
        };
#undef J_CREATE_ACTION

        static constexpr auto kActionCount_ = static_cast<int>(Type::count_);

        ///
        /// \return Function which performs (executes) given PlayerAction::Type
        J_NODISCARD static Executor& GetExecutor(Type type);

    private:
        // Executors for the actions

        static void PlayerMoveUp(GameDataGlobal& data_global);
        static void PlayerMoveRight(GameDataGlobal& data_global);
        static void PlayerMoveDown(GameDataGlobal& data_global);
        static void PlayerMoveLeft(GameDataGlobal& data_global);

        /// For test use only, sets player position to -100, 120
        static void ActionTest(GameDataGlobal& data_global);


#define J_CREATE_ACTION(enum_name__, executor__) [](auto& data_global) { executor__(data_global); },
        inline static std::array<Executor, kActionCount_> executors_{J_PLAYER_ACTIONS};
#undef J_CREATE_ACTION
    };

#undef J_PLAYER_ACTIONS

} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_PLAYER_PLAYER_ACTION_H
