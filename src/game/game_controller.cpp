// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/game_controller.h"

#include <filesystem>

#include "core/execution_timer.h"
#include "data/save_game_manager.h"
#include "game/event/game_events.h"
#include "game/logic/conveyor_controller.h"
#include "game/logic/inserter_controller.h"

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
    input.mouse.SkipErasingLastOverlay();       // Overlays
    player.placement.SetActivatedTile(nullptr); // ChunkTile
}

bool game::GameController::Init() {
    if (!InitPrototypes())
        return false;

    InitKeybinds();

    return true;
}

void game::GameController::LogicUpdate() {
    // World

    for (auto& world : worlds) {
        logic.GameTickAdvance();
        logic.DeferralUpdate(world, logic.GameTick());


        world.GenChunk(proto);


        // Logistics logic
        {
            EXECUTION_PROFILE_SCOPE(belt_timer, "Belt update");

            ConveyorLogicUpdate(world);
        }
        {
            EXECUTION_PROFILE_SCOPE(inserter_timer, "Inserter update");

            InserterLogicUpdate(world, logic);
        }
    }

    // Player

    input.mouse.DrawCursorOverlay(worlds, player, proto);

    player.crafting.RecipeCraftTick(proto);


    // World + player events

    event.Raise<LogicTickEvent>(EventType::logic_tick, logic.GameTick() % kGameHertz);
    input.key.Raise();
}

bool game::GameController::InitPrototypes() {
    try {
        proto.Load(data::PrototypeManager::kDataFolder);
        return true;
    }
    catch (proto::ProtoError&) {
        // Prototype loading error
        return false;
    }
    catch (std::filesystem::filesystem_error&) {
        // Data folder not found error
        LOG_MESSAGE_F(error, "data folder not found at %s", data::PrototypeManager::kDataFolder);
        return false;
    }
}

void game::GameController::InitKeybinds() {
    try {
        if (!data::DeserializeKeybinds(keybindManager)) {
            LOG_MESSAGE(warning, "No keybinds saved, using default keybinds");
            keybindManager.LoadDefaultKeybinds();
        }
        else {
            keybindManager.RegisterAllKeyData();
            LOG_MESSAGE(info, "Loaded keybinds from file");
        }
    }
    catch (std::exception& e) {
        LOG_MESSAGE_F(error, "Failed to load keybinds with message: %s, using default keybinds", e.what());
        keybindManager.LoadDefaultKeybinds();
    }
}
