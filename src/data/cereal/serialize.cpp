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

	const std::vector<std::function<void(game::GameDataGlobal* data_global)>> pre_load_hooks
	{
		[&](auto*) {
			data_local.prototype.GenerateRelocationTable();
			active_data_manager = &data_local.prototype;
		},
	};
	const std::vector<std::function<void(game::GameDataGlobal* data_global)>> post_load_hooks
	{
		[&](auto* data_global) {
			data_global->world.ResolveMultiTileTopLeft();
		},
	};


	// ======================================================================

	auto run_hooks = [](const std::vector<std::function<void(game::GameDataGlobal*)>>& hooks, const std::string& message,
	                    game::GameDataGlobal* data_global) {
		for (std::size_t i = 0; i < hooks.size(); ++i) {
			LOG_MESSAGE_F(debug, "%s %d of %d", message.c_str(), i + 1, hooks.size());

			const auto& hook = hooks[i];
			hook(data_global);
		}

		LOG_MESSAGE_F(debug, "%s Done", message.c_str());
	};


	run_hooks(pre_load_hooks, "Pre load hook", nullptr);

	std::ifstream in_cereal_stream(kSaveGameFileName, std::ios_base::binary);
	cereal::PortableBinaryInputArchive iarchive(in_cereal_stream);

	iarchive(out_data_global);

	run_hooks(post_load_hooks, "Post load hook", &out_data_global);

	LOG_MESSAGE(info, "Loading savegame Done");
}