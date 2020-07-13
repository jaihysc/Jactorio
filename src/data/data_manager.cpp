// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/22/2019

#include "data/data_manager.h"

#include <filesystem>
#include <sstream>

#include "core/filesystem.h"
#include "core/logger.h"
#include "data/local_parser.h"
#include "data/pybind/pybind_manager.h"

jactorio::data::PrototypeManager::~PrototypeManager() {
	ClearData();
}

void jactorio::data::PrototypeManager::SetDirectoryPrefix(const std::string& name) {
	directoryPrefix_ = name;
}

void jactorio::data::PrototypeManager::DataRawAdd(const std::string& iname,
                                             PrototypeBase* const prototype,
                                             const bool add_directory_prefix) {
	const DataCategory data_category = prototype->Category();

	// Use the following format internal name
	// Format __dir__/iname
	std::string formatted_iname;
	{
		std::ostringstream sstr;
		if (add_directory_prefix)
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
		const auto& category = dataRaw[static_cast<uint16_t>(data_category)];
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


	dataRaw[static_cast<uint16_t>(data_category)][formatted_iname] = prototype;
	LOG_MESSAGE_F(debug, "Added prototype %d %s", data_category, formatted_iname.c_str());
}

void jactorio::data::PrototypeManager::LoadData(
	const std::string& data_folder_path) {
	// Get all sub-folders in ~/data/
	// Read data.cfg files within each sub-folder
	// Load extracted data into loaded_data

	// Terminate the interpreter after loading prototypes
	auto py_guard = core::ResourceGuard(PyInterpreterTerminate);
	PyInterpreterInit();


	for (const auto& entry : std::filesystem::directory_iterator(data_folder_path)) {
		const std::string directory_name = entry.path().filename().u8string();

		// Directory including current folder: eg: /data/base
		std::string current_directory;
		{
			std::stringstream ss;
			ss << data_folder_path << "/" << directory_name;
			current_directory = ss.str();
		}

		// Python file
		{
			std::stringstream py_file_path;
			py_file_path << current_directory << "/data.py";

			const std::string py_file_contents = core::ReadFile(py_file_path.str());
			// data.py file does not exist
			if (py_file_contents.empty()) {
				LOG_MESSAGE_F(warning, "Directory %s has no or empty data.py file. Ignoring", current_directory.c_str());
				continue;
			}


			SetDirectoryPrefix(directory_name);
			try {
				PyExec(py_file_contents, py_file_path.str());
			}
			catch (DataException& e) {
				LOG_MESSAGE_F(error, "%s", e.what());
				throw;
			}
		}


		// Load local file for the directory
		{
			std::stringstream cfg_file_path;
			// TODO selectable language
			cfg_file_path << current_directory << "/local/" <<
				kLanguageIdentifier[static_cast<int>(Language::en)] << ".cfg";

			if (!std::filesystem::exists(cfg_file_path.str())) {
				LOG_MESSAGE_F(warning,
				              "Directory %s missing local at %s",
				              current_directory.c_str(),
				              cfg_file_path.str().c_str());
				continue;
			}

			auto local_contents = core::ReadFile(cfg_file_path.str());
			LocalParseNoThrow(*this, local_contents, directory_name);
		}

		LOG_MESSAGE_F(info, "Directory %s loaded", current_directory.c_str());
	}

	LOG_MESSAGE(info, "Validating loaded prototypes");
	for (auto& prototype_categories : dataRaw) {
		for (auto& pair : prototype_categories) {
			auto& prototype = *pair.second;
			LOG_MESSAGE_F(debug, "Validating prototype %d %s", prototype.internalId, prototype.name.c_str());

			prototype.PostLoad();
			try {
				prototype.PostLoadValidate(*this);
			}
			catch (DataException& e) {
				LOG_MESSAGE_F(error, "Prototype validation failed: '%s'", e.what());
				throw;
			}
			prototype.ValidatedPostLoad();

			LOG_MESSAGE_F(debug, "Validating prototype %d %s Success\n", prototype.internalId, prototype.name.c_str());
		}
	}
}

bool jactorio::data::PrototypeManager::PrototypeExists(const std::string& iname) const {
	for (const auto& map : dataRaw) {
		if (map.find(iname) != map.end()) {
				return true;
		}
	}
	return false;
}

void jactorio::data::PrototypeManager::ClearData() {
	// Iterate through both unordered maps and delete all pointers
	for (auto& map : dataRaw) {
		// Category unordered maps
		for (auto& category_pair : map) {
			delete category_pair.second;
		}
		map.clear();
	}

	internalIdNew_ = internal_id_start;
}
