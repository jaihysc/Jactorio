// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/player/player_action.h"

#include "game/game_controller.h"
#include "gui/imgui_manager.h"
#include "gui/menus.h"
#include "render/render_controller.h"

using namespace jactorio;

game::PlayerAction::Executor& game::PlayerAction::GetExecutor(const Type type) {
    assert(type != Type::count_);

    return executors_[static_cast<int>(type)];
}

static float GetMovementSpeed(const game::PlayerAction::Context& c) {
    constexpr float max_move_speed = 30.f;
    return max_move_speed * (1.f - c.RenderController().renderer.GetZoom() + 0.01f); // Never 0
}

void game::PlayerAction::PlayerMoveUp(const Context& c) {
    c.GameController().player.world.MovePlayerY(GetMovementSpeed(c) * -1);
}

void game::PlayerAction::PlayerMoveRight(const Context& c) {
    c.GameController().player.world.MovePlayerX(GetMovementSpeed(c));
}

void game::PlayerAction::PlayerMoveDown(const Context& c) {
    c.GameController().player.world.MovePlayerY(GetMovementSpeed(c));
}

void game::PlayerAction::PlayerMoveLeft(const Context& c) {
    c.GameController().player.world.MovePlayerX(GetMovementSpeed(c) * -1);
}


void game::PlayerAction::DeselectHeldItem(const Context& c) {
    c.GameController().player.inventory.DeselectSelectedItem();
}


void game::PlayerAction::PlaceEntity(const Context& c) {
    if (gui::input_mouse_captured || !c.GameController().player.world.MouseSelectedTileInRange())
        return;

    const auto tile_selected_coord = c.GameController().player.world.GetMouseTileCoords();

    auto& player = c.GameController().player;
    auto& world  = c.GameController().worlds[player.world.GetId()];

    player.placement.TryPlaceEntity(world, c.GameController().logic, tile_selected_coord);
}

void game::PlayerAction::ActivateTile(const Context& c) {
    if (gui::input_mouse_captured || !c.GameController().player.world.MouseSelectedTileInRange())
        return;

    auto& player = c.GameController().player;
    auto& world  = c.GameController().worlds[player.world.GetId()];

    player.placement.TryActivateTile(world, player.world.GetMouseTileCoords());
}

void game::PlayerAction::PickupOrMineEntity(const Context& c) {
    if (gui::input_mouse_captured || !c.GameController().player.world.MouseSelectedTileInRange())
        return;

    const auto tile_selected_coord = c.GameController().player.world.GetMouseTileCoords();

    auto& player = c.GameController().player;
    auto& world  = c.GameController().worlds[player.world.GetId()];

    player.placement.TryPickup(world, c.GameController().logic, tile_selected_coord);
}


void game::PlayerAction::RotateEntityClockwise(const Context& c) {
    c.GameController().player.placement.RotateOrientation();
}
void game::PlayerAction::RotateEntityCounterClockwise(const Context& c) {
    c.GameController().player.placement.CounterRotateOrientation();
}


void game::PlayerAction::ToggleMainMenu(const Context& /*c*/) {
    SetVisible(gui::Menu::MainMenu, !IsVisible(gui::Menu::MainMenu));
}

void game::PlayerAction::ToggleDebugMenu(const Context& /*c*/) {
    SetVisible(gui::Menu::DebugMenu, !IsVisible(gui::Menu::DebugMenu));
}

void game::PlayerAction::ToggleCharacterMenu(const Context& c) {
    // If a tile is already activated, deactivate it, otherwise open the gui menu
    if (c.GameController().player.placement.GetActivatedTile() != nullptr)
        c.GameController().player.placement.SetActivatedTile(nullptr);
    else
        SetVisible(gui::Menu::CharacterMenu, !IsVisible(gui::Menu::CharacterMenu));
}


void game::PlayerAction::ActionTest(const Context& c) {
    c.GameController().player.world.SetPlayerX(-100.f);
    c.GameController().player.world.SetPlayerY(120.f);
}
