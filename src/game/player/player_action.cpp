// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/player/player_action.h"

#include "game/game_controller.h"
#include "gui/imgui_manager.h"
#include "gui/menus.h"

using namespace jactorio;

game::PlayerAction::Executor& game::PlayerAction::GetExecutor(const Type type) {
    assert(type != Type::count_);

    return executors_[static_cast<int>(type)];
}

constexpr float kMoveSpeed = 0.8f;

void game::PlayerAction::PlayerMoveUp(GameController& game_controller) {
    game_controller.player.world.MovePlayerY(kMoveSpeed * -1);
}

void game::PlayerAction::PlayerMoveRight(GameController& game_controller) {
    game_controller.player.world.MovePlayerX(kMoveSpeed);
}

void game::PlayerAction::PlayerMoveDown(GameController& game_controller) {
    game_controller.player.world.MovePlayerY(kMoveSpeed);
}

void game::PlayerAction::PlayerMoveLeft(GameController& game_controller) {
    game_controller.player.world.MovePlayerX(kMoveSpeed * -1);
}


void game::PlayerAction::DeselectHeldItem(GameController& game_controller) {
    game_controller.player.inventory.DeselectSelectedItem();
}


void game::PlayerAction::PlaceEntity(GameController& game_controller) {
    if (gui::input_mouse_captured || !game_controller.player.world.MouseSelectedTileInRange())
        return;

    const auto tile_selected_coord = game_controller.player.world.GetMouseTileCoords();

    auto& player = game_controller.player;
    auto& world  = game_controller.worlds[player.world.GetId()];

    player.placement.TryPlaceEntity(world, game_controller.logic, tile_selected_coord);
}

void game::PlayerAction::ActivateLayer(GameController& game_controller) {
    if (gui::input_mouse_captured || !game_controller.player.world.MouseSelectedTileInRange())
        return;

    auto& player = game_controller.player;
    auto& world  = game_controller.worlds[player.world.GetId()];

    player.placement.TryActivateLayer(world, player.world.GetMouseTileCoords());
}

void game::PlayerAction::PickupOrMineEntity(GameController& game_controller) {
    if (gui::input_mouse_captured || !game_controller.player.world.MouseSelectedTileInRange())
        return;

    const auto tile_selected_coord = game_controller.player.world.GetMouseTileCoords();

    auto& player = game_controller.player;
    auto& world  = game_controller.worlds[player.world.GetId()];

    player.placement.TryPickup(world, game_controller.logic, tile_selected_coord);
}


void game::PlayerAction::RotateEntityClockwise(GameController& game_controller) {
    game_controller.player.placement.RotateOrientation();
}
void game::PlayerAction::RotateEntityCounterClockwise(GameController& game_controller) {
    game_controller.player.placement.CounterRotateOrientation();
}


void game::PlayerAction::ToggleMainMenu(GameController& /*game_controller*/) {
    SetVisible(gui::Menu::MainMenu, !IsVisible(gui::Menu::MainMenu));
}

void game::PlayerAction::ToggleDebugMenu(GameController& /*game_controller*/) {
    SetVisible(gui::Menu::DebugMenu, !IsVisible(gui::Menu::DebugMenu));
}

void game::PlayerAction::ToggleCharacterMenu(GameController& game_controller) {
    // If a layer is already activated, deactivate it, otherwise open the gui menu
    if (game_controller.player.placement.GetActivatedLayer() != nullptr)
        game_controller.player.placement.SetActivatedLayer(nullptr);
    else
        SetVisible(gui::Menu::CharacterMenu, !IsVisible(gui::Menu::CharacterMenu));
}


void game::PlayerAction::ActionTest(GameController& game_controller) {
    game_controller.player.world.SetPlayerX(-100.f);
    game_controller.player.world.SetPlayerY(120.f);
}
