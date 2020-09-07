// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "data/cereal/serialize.h"

#include <fstream>
#include <functional>

#include "data/cereal/register_type.h"
#include "game/game_data.h"

#include <cereal/archives/portable_binary.hpp>

using namespace jactorio;

static constexpr auto kSaveGameFileName = "savegame.dat";

void data::SerializeGameData(const game::GameDataGlobal& game_data) {
    LOG_MESSAGE(info, "Saving savegame");

    std::ofstream out_cereal_stream(kSaveGameFileName, std::ios_base::binary);
    cereal::PortableBinaryOutputArchive output_archive(out_cereal_stream);
    output_archive(game_data);

    LOG_MESSAGE(info, "Saving savegame Done");
}

void data::DeserializeGameData(game::GameDataLocal& data_local, game::GameDataGlobal& out_data_global) {
    LOG_MESSAGE(info, "Loading savegame");

    const std::vector<std::function<void()>> pre_load_hooks{
        [&]() {
            data_local.prototype.GenerateRelocationTable();
            active_prototype_manager = &data_local.prototype;
        },
        [&]() {
            data_local.input.mouse.SkipErasingLastOverlay(); // All overlays will be cleared
        },
        [&]() {
            out_data_global.player.placement.SetActivatedLayer(nullptr); // Prevents gui accessing non existent data
        },
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

    std::ifstream in_cereal_stream(kSaveGameFileName, std::ios_base::binary);
    cereal::PortableBinaryInputArchive iarchive(in_cereal_stream);

    iarchive(out_data_global);

    run_hooks(post_load_hooks, "Post load hook");

    LOG_MESSAGE(info, "Loading savegame Done");
}
