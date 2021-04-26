// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "data/save_game_manager.h"

using namespace jactorio;

static constexpr auto kSaveGameFolder  = "saves";
static constexpr auto kSaveGameFileExt = "dat";

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
