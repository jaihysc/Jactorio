// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/game_controller.h"

using namespace jactorio;

void game::GameController::ResetGame() {
    ClearRefsToWorld();
    worlds.~GameWorlds();
    logic.~Logic();
    player.~Player();

    new (&worlds) GameWorlds(kDefaultWorldCount);
    new (&logic) Logic();
    new (&player) Player();
}

void game::GameController::ClearRefsToWorld() {
    input.mouse.SkipErasingLastOverlay();        // Overlays
    player.placement.SetActivatedLayer(nullptr); // ChunkTileLayer
}
