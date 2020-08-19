// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "data/cereal/serialize.h"

#include <fstream>
#include <cereal/archives/portable_binary.hpp>

#include "data/cereal/register_type.h"
#include "game/game_data.h"

static constexpr auto kSaveGameFileName = "savegame.dat";

// void jactorio::data::SerializeGameData(const game::GameDataGlobal& game_data) {
// 	LOG_MESSAGE(info, "Saving savegame");
//
// 	std::ofstream out_cereal_stream(kSaveGameFileName, std::ios_base::binary);
// 	cereal::PortableBinaryOutputArchive output_archive(out_cereal_stream);
// 	output_archive(game_data);
// }
//
// jactorio::game::GameDataGlobal jactorio::data::DeserializeGameData() {
// 	LOG_MESSAGE(info, "Loading savegame");
// 	// game_data->prototype.GenerateRelocationTable();
//
// 	std::ifstream in_cereal_stream(kSaveGameFileName, std::ios_base::binary);
// 	cereal::PortableBinaryInputArchive iarchive(in_cereal_stream);
//
// 	game::GameDataGlobal m;
// 	iarchive(m);
// 	return m; // TODO make GameDataGlobal copy and movable
// }
