// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "game/game_controller.h"

#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>

#include "core/execution_timer.h"
#include "data/cereal/register_type.h"
#include "data/save_game_manager.h"
#include "game/event/game_events.h"
#include "game/logic/conveyor_controller.h"
#include "game/logic/inserter_controller.h"

using namespace jactorio;

void game::GameController::ResetGame() {
    worlds.~GameWorlds();
    logic.~Logic();
    player.~Player();

    new (&worlds) GameWorlds(kDefaultWorldCount);
    new (&logic) Logic();
    new (&player) Player();
}

bool game::GameController::Init() {
    if (!InitPrototypes())
        return false;

    try {
        LoadSetting();

        keybindManager.RegisterAllKeyData();
    }
    catch (std::runtime_error& e) {
        LOG_MESSAGE_F(error, "Load settings error: %s", e.what());

        // Must recover so user can at least make it to the main menu to fix settings

        LOG_MESSAGE(warning, "Using default keybinds, default localization");
        keybindManager.LoadDefaultKeybinds();

        try {
            SaveSetting();
        }
        catch (std::runtime_error& e_2) {
            LOG_MESSAGE_F(error, "Failed to save default settings file: %s", e_2.what());
        }
    }

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

void game::GameController::SaveSetting() const {
    const auto* save_path = kSettingsPath;

    LOG_MESSAGE_F(info, "Saving settings to '%s'", save_path);

    std::ofstream ofs(save_path);
    cereal::JSONOutputArchive archiver(ofs);

    // Output archive guaranteed to not modify
    const_cast<GameController*>(this)->SerializeSetting(archiver);
}

void game::GameController::LoadSetting() {
    const auto* save_path = kSettingsPath;

    LOG_MESSAGE_F(info, "Loading settings from '%s'", save_path);

    if (!std::filesystem::exists(save_path)) {
        throw std::runtime_error("Setting file does not exist");
    }

    std::ifstream ifs(save_path);
    cereal::JSONInputArchive archiver(ifs);

    SerializeSetting(archiver);
}

void game::GameController::SaveGame(const char* save_name) const {
    const auto save_path = data::ResolveSavePath(save_name);
    LOG_MESSAGE_F(info, "Saving game to '%s'", save_path.c_str());

    std::ofstream ofs(save_path.c_str(), std::ios_base::binary);
    cereal::PortableBinaryOutputArchive archive(ofs);

    // Output archive guaranteed to not modify
    const_cast<GameController*>(this)->SerializeGame(archive);
}

void game::GameController::LoadGame(const char* save_name) {
    const auto save_path = data::ResolveSavePath(save_name);
    LOG_MESSAGE_F(info, "Loading save from '%s'", save_path.c_str());

    if (!std::filesystem::exists(save_path)) {
        throw std::runtime_error("Save file does not exist");
    }

    const std::vector<std::function<void()>> pre_load_hooks{
        [&]() {
            proto.GenerateRelocationTable();
            data::active_prototype_manager = &proto;
        },
        [&]() { ResetGame(); }, // Remove any dangling pointers (activatedTile)
    };
    const std::vector<std::function<void()>> post_load_hooks{
        [&]() {
            for (auto& world : worlds) {
                world.DeserializePostProcess();
            }
        },
        [&]() { unique.Clear(); },
    };


    // ======================================================================

    auto run_hooks = [](const std::vector<std::function<void()>>& hooks, const std::string& message) {
        for (std::size_t i = 0; i < hooks.size(); ++i) {
            LOG_MESSAGE_F(debug, "%s %d of %d", message.c_str(), i + 1, hooks.size());

            const auto& hook = hooks[i];
            hook();
        }

        LOG_MESSAGE_F(debug, "%s Done", message.c_str());
    };


    run_hooks(pre_load_hooks, "Pre load hook");

    std::ifstream ifs(save_path.c_str(), std::ios_base::binary);
    cereal::PortableBinaryInputArchive archive(ifs);

    SerializeGame(archive);

    run_hooks(post_load_hooks, "Post load hook");
}

// ======================================================================

bool game::GameController::InitPrototypes() {
    try {
        proto.LoadProto(data::PrototypeManager::kDataFolder);
        proto.LoadLocal(data::PrototypeManager::kDataFolder, localIdentifier.c_str());
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
