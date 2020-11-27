// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/player/player_action.h"

#include "game/game_data.h"
#include "gui/imgui_manager.h"
#include "gui/menus.h"

using namespace jactorio;

game::PlayerAction::Executor& game::PlayerAction::GetExecutor(const Type type) {
    assert(type != Type::count_);

    return executors_[static_cast<int>(type)];
}

constexpr float kMoveSpeed = 0.8f;

void game::PlayerAction::PlayerMoveUp(GameDataGlobal& data_global) {
    data_global.player.world.MovePlayerY(kMoveSpeed * -1);
}

void game::PlayerAction::PlayerMoveRight(GameDataGlobal& data_global) {
    data_global.player.world.MovePlayerX(kMoveSpeed);
}

void game::PlayerAction::PlayerMoveDown(GameDataGlobal& data_global) {
    data_global.player.world.MovePlayerY(kMoveSpeed);
}

void game::PlayerAction::PlayerMoveLeft(GameDataGlobal& data_global) {
    data_global.player.world.MovePlayerX(kMoveSpeed * -1);
}


void game::PlayerAction::DeselectHeldItem(GameDataGlobal& data_global) {
    data_global.player.inventory.DeselectSelectedItem();
}


void game::PlayerAction::PlaceEntity(GameDataGlobal& data_global) {
    if (gui::input_mouse_captured || !data_global.player.world.MouseSelectedTileInRange())
        return;

    const auto tile_selected = data_global.player.world.GetMouseTileCoords();

    auto& player = data_global.player;
    auto& world  = data_global.worlds[player.world.GetId()];

    player.placement.TryPlaceEntity(world, data_global.logic, tile_selected.x, tile_selected.y);
}

void game::PlayerAction::ActivateLayer(GameDataGlobal& data_global) {
    if (gui::input_mouse_captured || !data_global.player.world.MouseSelectedTileInRange())
        return;

    auto& player = data_global.player;
    auto& world  = data_global.worlds[player.world.GetId()];

    player.placement.TryActivateLayer(world, player.world.GetMouseTileCoords());
}

void game::PlayerAction::PickupOrMineEntity(GameDataGlobal& data_global) {
    if (gui::input_mouse_captured || !data_global.player.world.MouseSelectedTileInRange())
        return;

    const auto tile_selected = data_global.player.world.GetMouseTileCoords();

    auto& player = data_global.player;
    auto& world  = data_global.worlds[player.world.GetId()];

    player.placement.TryPickup(world, data_global.logic, tile_selected.x, tile_selected.y);
}


void game::PlayerAction::RotateEntityClockwise(GameDataGlobal& data_global) {
    data_global.player.placement.RotateOrientation();
}
void game::PlayerAction::RotateEntityCounterClockwise(GameDataGlobal& data_global) {
    data_global.player.placement.CounterRotateOrientation();
}


void game::PlayerAction::ToggleMainMenu(GameDataGlobal& /*data_global*/) {
    SetVisible(gui::Menu::MainMenu, !IsVisible(gui::Menu::MainMenu));
}

void game::PlayerAction::ToggleDebugMenu(GameDataGlobal& /*data_global*/) {
    SetVisible(gui::Menu::DebugMenu, !IsVisible(gui::Menu::DebugMenu));
}

void game::PlayerAction::ToggleCharacterMenu(GameDataGlobal& data_global) {
    // If a layer is already activated, deactivate it, otherwise open the gui menu
    if (data_global.player.placement.GetActivatedLayer() != nullptr)
        data_global.player.placement.SetActivatedLayer(nullptr);
    else
        SetVisible(gui::Menu::CharacterMenu, !IsVisible(gui::Menu::CharacterMenu));
}


void game::PlayerAction::ActionTest(GameDataGlobal& data_global) {
    data_global.player.world.SetPlayerX(-100.f);
    data_global.player.world.SetPlayerY(120.f);
}
