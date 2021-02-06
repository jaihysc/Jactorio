// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "data/save_game_manager.h"

#include <filesystem>
#include <fstream>
#include <functional>

#include "data/cereal/register_type.h"
#include "game/game_data.h"
#include "game/player/keybind_manager.h"

#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>

using namespace jactorio;

static constexpr auto kSaveGameFolder  = "saves";
static constexpr auto kSaveGameFileExt = "dat";

void data::SerializeGameData(const game::GameDataGlobal& game_data, const std::string& save_name) {
    LOG_MESSAGE_F(info, "Saving savegame as %s", save_name.c_str());

    std::ofstream out_cereal_stream(ResolveSavePath(save_name), std::ios_base::binary);
    cereal::PortableBinaryOutputArchive output_archive(out_cereal_stream);
    output_archive(game_data);

    LOG_MESSAGE(info, "Saving savegame Done");
}

void data::DeserializeGameData(game::GameDataLocal& data_local,
                               game::GameDataGlobal& out_data_global,
                               const std::string& save_name) {
    LOG_MESSAGE_F(info, "Loading savegame %s", save_name.c_str());

    const std::vector<std::function<void()>> pre_load_hooks{
        [&]() {
            data_local.proto.GenerateRelocationTable();
            active_prototype_manager = &data_local.proto;
        },
        [&]() { out_data_global.ClearRefsToWorld(data_local); },
    };
    const std::vector<std::function<void()>> post_load_hooks{
        [&]() {
            for (auto& world : out_data_global.worlds) {
                world.DeserializePostProcess();
            }
        },
        [&]() { data_local.unique.RelocationClear(); },
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

    std::ifstream in_cereal_stream(ResolveSavePath(save_name), std::ios_base::binary);
    cereal::PortableBinaryInputArchive iarchive(in_cereal_stream);

    iarchive(out_data_global);

    run_hooks(post_load_hooks, "Post load hook");

    LOG_MESSAGE(info, "Loading savegame Done");
}

bool data::IsValidSaveName(const std::string& save_name) {
    const auto path = std::filesystem::path(save_name);

    // Backslash (\) disallowed on all platforms to maintain file portability with Windows
    if (path.string().find("\\") != std::string::npos)
        return false;

    if (path.has_root_path())
        return false;
    if (path.has_root_name())
        return false;
    if (path.has_root_directory())
        return false;
    if (!path.has_relative_path())
        return false;
    if (path.has_parent_path())
        return false;
    if (!path.has_stem())
        return false;
    if (!path.has_filename())
        return false;
    if (path.has_extension())
        return false;

    return true;
}


///
/// If save directory does not exist, a directory is made
void CheckExistsSaveDirectory() {
    if (!std::filesystem::exists(kSaveGameFolder)) {
        std::filesystem::create_directory(kSaveGameFolder);
    }
}

std::string data::ResolveSavePath(const std::string& save_name) {
    assert(IsValidSaveName(save_name));

    CheckExistsSaveDirectory();
    return std::string(kSaveGameFolder) + "/" + save_name + "." + kSaveGameFileExt;
}

std::filesystem::directory_iterator data::GetSaveDirIt() {
    CheckExistsSaveDirectory();
    return std::filesystem::directory_iterator(kSaveGameFolder);
}

// ======================================================================

void data::SerializeKeybinds(const game::KeybindManager& keybind_manager) {
    LOG_MESSAGE_F(info, "Saving keybinds to %s", kKeybindSaveName);

    std::ofstream of(kKeybindSaveName);
    cereal::JSONOutputArchive archiver(of);

    archiver(keybind_manager);
}

bool data::DeserializeKeybinds(game::KeybindManager& out_keybind_manager) {
    LOG_MESSAGE_F(info, "Loading keybinds from %s", kKeybindSaveName);

    if (!std::filesystem::exists(kKeybindSaveName))
        return false;

    std::ifstream ifs(kKeybindSaveName);
    cereal::JSONInputArchive archiver(ifs);

    archiver(out_keybind_manager);

    return true;
}
