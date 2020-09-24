// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_DATA_SAVE_GAME_MANAGER_H
#define JACTORIO_INCLUDE_DATA_SAVE_GAME_MANAGER_H
#pragma once

#include <filesystem>

#include "jactorio.h"


namespace jactorio::game
{
    struct GameDataLocal;
    struct GameDataGlobal;
} // namespace jactorio::game

namespace jactorio::data
{
    ///
    /// Must be called before world data is cleared
    void PrepareWorldDataClear(game::GameDataLocal& data_local, game::GameDataGlobal& data_global);


    void SerializeGameData(const game::GameDataGlobal& game_data, const std::string& save_name);
    ///
    /// \param out_data_global Deserialized into this
    void DeserializeGameData(game::GameDataLocal& data_local,
                             game::GameDataGlobal& out_data_global,
                             const std::string& save_name);

    J_NODISCARD bool IsValidSaveName(const std::string& save_name);

    ///
    /// \param save_name No path, no extensions
    J_NODISCARD std::string ResolveSavePath(const std::string& save_name);

    ///
    /// Iterator to save directory, directory itself is always valid
    J_NODISCARD std::filesystem::directory_iterator GetSaveDirIt();
} // namespace jactorio::data

#endif // JACTORIO_INCLUDE_DATA_SAVE_GAME_MANAGER_H
