// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_PLAYER_PLAYER_ACTION_H
#define JACTORIO_INCLUDE_GAME_PLAYER_PLAYER_ACTION_H
#pragma once

#include <array>

#include "jactorio.h"

namespace jactorio::game
{
    class GameController;

    /// Associates enum key (action) with method (executor)
    /// \remark Do NOT reorder as order here is referenced by localized names in local files
#define J_PLAYER_ACTIONS                                                           \
    J_CREATE_ACTION(player_move_up, PlayerMoveUp)                                  \
    J_CREATE_ACTION(player_move_right, PlayerMoveRight)                            \
    J_CREATE_ACTION(player_move_down, PlayerMoveDown)                              \
    J_CREATE_ACTION(player_move_left, PlayerMoveLeft)                              \
                                                                                   \
    J_CREATE_ACTION(deselect_held_item, DeselectHeldItem)                          \
                                                                                   \
    J_CREATE_ACTION(place_entity, PlaceEntity)                                     \
    J_CREATE_ACTION(activate_layer, ActivateTile)                                  \
    J_CREATE_ACTION(pickup_or_mine_entity, PickupOrMineEntity)                     \
                                                                                   \
    J_CREATE_ACTION(rotate_entity_clockwise, RotateEntityClockwise)                \
    J_CREATE_ACTION(rotate_entity_counter_clockwise, RotateEntityCounterClockwise) \
                                                                                   \
    J_CREATE_ACTION(toggle_main_menu, ToggleMainMenu)                              \
    J_CREATE_ACTION(toggle_debug_menu, ToggleDebugMenu)                            \
    J_CREATE_ACTION(toggle_character_menu, ToggleCharacterMenu)                    \
                                                                                   \
    J_CREATE_ACTION(test, ActionTest)

    struct PlayerAction
    {
        using Executor = void (*)(GameController& game_controller);

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

        static void PlayerMoveUp(GameController& game_controller);
        static void PlayerMoveRight(GameController& game_controller);
        static void PlayerMoveDown(GameController& game_controller);
        static void PlayerMoveLeft(GameController& game_controller);

        static void DeselectHeldItem(GameController& game_controller);

        static void PlaceEntity(GameController& game_controller);
        static void ActivateTile(GameController& game_controller);
        static void PickupOrMineEntity(GameController& game_controller);

        static void RotateEntityClockwise(GameController& game_controller);
        static void RotateEntityCounterClockwise(GameController& game_controller);

        static void ToggleMainMenu(GameController& game_controller);
        static void ToggleDebugMenu(GameController& game_controller);
        static void ToggleCharacterMenu(GameController& game_controller);

        /// For test use only, sets player position to -100, 120
        static void ActionTest(GameController& game_controller);


#define J_CREATE_ACTION(enum_name__, executor__) [](auto& game_controller) { executor__(game_controller); },
        inline static std::array<Executor, kActionCount_> executors_{J_PLAYER_ACTIONS};
#undef J_CREATE_ACTION
    };

#undef J_PLAYER_ACTIONS

} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_PLAYER_PLAYER_ACTION_H
