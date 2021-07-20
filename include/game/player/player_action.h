// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_PLAYER_PLAYER_ACTION_H
#define JACTORIO_INCLUDE_GAME_PLAYER_PLAYER_ACTION_H
#pragma once

#include <array>
#include <functional>

#include "jactorio.h"

namespace jactorio::render
{
    class RenderController;
}
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
        struct Context
        {
            std::reference_wrapper<GameController> gameController;
            /// Handle to renderer, since initialization of the renderer is delayed
            render::RenderController** hRenderController = nullptr;

            // Hide implementation details on how the members are obtained
            J_NODISCARD GameController& GameController() const noexcept {
                return gameController.get();
            }
            J_NODISCARD render::RenderController& RenderController() const noexcept {
                assert(hRenderController != nullptr);
                assert(*hRenderController != nullptr);
                return **hRenderController;
            }
        };
        using Executor = void (*)(const Context& c);

#define J_CREATE_ACTION(enum_name__, executor__) enum_name__,
        enum class Type
        {
            J_PLAYER_ACTIONS

                count_
        };
#undef J_CREATE_ACTION

        static constexpr auto kActionCount_ = static_cast<int>(Type::count_);

        /// \return Function which performs (executes) given PlayerAction::Type
        J_NODISCARD static Executor& GetExecutor(Type type);

    private:
        // Executors for the actions

        static void PlayerMoveUp(const Context& c);
        static void PlayerMoveRight(const Context& c);
        static void PlayerMoveDown(const Context& c);
        static void PlayerMoveLeft(const Context& c);

        static void DeselectHeldItem(const Context& c);

        static void PlaceEntity(const Context& c);
        static void ActivateTile(const Context& c);
        static void PickupOrMineEntity(const Context& c);

        static void RotateEntityClockwise(const Context& c);
        static void RotateEntityCounterClockwise(const Context& c);

        static void ToggleMainMenu(const Context& c);
        static void ToggleDebugMenu(const Context& c);
        static void ToggleCharacterMenu(const Context& c);

        /// For test use only, sets player position to -100, 120
        static void ActionTest(const Context& c);


#define J_CREATE_ACTION(enum_name__, executor__) [](auto& game_controller) { executor__(game_controller); },
        inline static std::array<Executor, kActionCount_> executors_{J_PLAYER_ACTIONS};
#undef J_CREATE_ACTION
    };

#undef J_PLAYER_ACTIONS

} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_PLAYER_PLAYER_ACTION_H
