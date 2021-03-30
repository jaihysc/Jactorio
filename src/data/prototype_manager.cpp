// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "data/prototype_manager.h"

#include <filesystem>
#include <sstream>

#include "core/filesystem.h"
#include "core/resource_guard.h"
#include "data/local_parser.h"
#include "data/pybind_manager.h"

using namespace jactorio;

data::PrototypeManager::~PrototypeManager() {
    Clear();
}

void data::PrototypeManager::SetDirectoryPrefix(const std::string& name) {
    directoryPrefix_ = name;
}

void data::PrototypeManager::Add(const std::string& iname, proto::FrameworkBase* const prototype) {
    const auto data_category = prototype->GetCategory();

    // Use the following format internal name
    // Format __dir__/iname
    std::string formatted_iname;
    {
        std::ostringstream sstr;
        if (!directoryPrefix_.empty())
            sstr << "__" << directoryPrefix_ << "__/";

        sstr << iname;
        formatted_iname = sstr.str();
    }

    // Print warning if overriding another name
    // Do not print warning in iname is empty and assign a new unique name
    if (iname.empty()) {
        // Generate a internal name based on the id
        std::ostringstream sstr;
        sstr << "@" << internalIdNew_;
        formatted_iname = sstr.str();
    }
    else {
        const auto& category = dataRaw_[static_cast<uint16_t>(data_category)];
        auto it              = category.find(formatted_iname);
        if (it != category.end()) {
            LOG_MESSAGE_F(warning,
                          "Name \"%s\" type %d overrides previous declaration",
                          formatted_iname.c_str(),
                          static_cast<int>(data_category));
            // Free the previous prototype
            delete it->second;
        }
    }

    // Enforce prototype rules...
    // No order specified, use internal id as order
    prototype->name = formatted_iname;

    if (prototype->order == 0)
        prototype->order = internalIdNew_;
    if (prototype->GetLocalizedName().empty())
        prototype->SetLocalizedName(formatted_iname);

    prototype->internalId = internalIdNew_++;


    dataRaw_[static_cast<uint16_t>(data_category)][formatted_iname] = prototype;
    LOG_MESSAGE_F(debug, "Added prototype %d %s", data_category, formatted_iname.c_str());
}

void data::PrototypeManager::LoadProto(const char* data_folder_path) {
    auto py_guard = ResourceGuard(PyInterpreterTerminate);
    PyInterpreterInit();

    for (const auto& entry : std::filesystem::directory_iterator(data_folder_path)) {
        const auto current_dir_name = entry.path().filename().u8string();
        const auto current_dir_path = std::string(data_folder_path) + "/" + current_dir_name;

        const auto py_file_path = current_dir_path + "/data.py";

        const auto py_file_contents = ReadFile(py_file_path);
        if (py_file_contents.empty()) {
            LOG_MESSAGE_F(warning,
                          "Directory '%s' has no or empty data.py file at '%s'",
                          current_dir_path.c_str(),
                          py_file_path.c_str());
            continue;
        }

        SetDirectoryPrefix(current_dir_name);
        try {
            PyExec(py_file_contents, py_file_path);
        }
        catch (proto::ProtoError& e) {
            LOG_MESSAGE_F(error, "%s", e.what());
            throw;
        }

        LOG_MESSAGE_F(info, "Directory '%s' prototype loaded", current_dir_path.c_str());
    }

    LOG_MESSAGE(info, "Validating loaded prototypes");
    for (auto& prototype_categories : dataRaw_) {
        for (auto& pair : prototype_categories) {
            auto& prototype = *pair.second;
            LOG_MESSAGE_F(debug, "Validating prototype %d %s", prototype.internalId, prototype.name.c_str());

            prototype.PostLoad();
            try {
                prototype.PostLoadValidate(*this);
            }
            catch (proto::ProtoError& e) {
                LOG_MESSAGE_F(error, "Prototype validation failed: '%s'", e.what());
                throw;
            }
            prototype.ValidatedPostLoad();

            LOG_MESSAGE_F(debug, "Validating prototype %d %s Success\n", prototype.internalId, prototype.name.c_str());
        }
    }
}

void data::PrototypeManager::LoadLocal(const char* data_folder_path, const char* local_identifier) {
    for (const auto& entry : std::filesystem::directory_iterator(data_folder_path)) {
        const auto current_dir_name = entry.path().filename().u8string();
        const auto current_dir_path = std::string(data_folder_path) + "/" + current_dir_name;

        const auto local_file_path = current_dir_path + "/local/" + local_identifier + ".cfg";

        if (!std::filesystem::exists(local_file_path)) {
            LOG_MESSAGE_F(
                warning, "Directory '%s' missing local at '%s'", current_dir_path.c_str(), local_file_path.c_str());
            continue;
        }

        auto local_contents = ReadFile(local_file_path);
        try {
            LocalParse(*this, local_contents, current_dir_name);
        }
        catch (proto::ProtoError& e) {
            LOG_MESSAGE_F(error, "%s", e.what()); // Do not rethrow. Recover in case of localization errors
        }

        LOG_MESSAGE_F(info, "Directory '%s' localization loaded", current_dir_path.c_str());
    }
}

void data::PrototypeManager::Clear() {
    // Iterate through both unordered maps and delete all pointers
    for (auto& map : dataRaw_) {
        // Category unordered maps
        for (auto& category_pair : map) {
            delete category_pair.second;
        }
        map.clear();
    }

    internalIdNew_ = kInternalIdStart_;
}


void data::PrototypeManager::GenerateRelocationTable() {
    relocationTable_.resize(internalIdNew_ - 1);
    for (auto& map : dataRaw_) {
        for (auto& [iname, pointer] : map) {
            assert(pointer != nullptr);
            // Internal id starts at 1, vector indexes from 0
            relocationTable_[pointer->internalId - 1] = pointer;
        }
    }
}

data::PrototypeManager::DebugInfo data::PrototypeManager::GetDebugInfo() const {
    return {
        relocationTable_,
    };
}
