// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/22/2019

#include "data/data_manager.h"

#include <filesystem>
#include <sstream>

#include "core/filesystem.h"
#include "core/logger.h"
#include "data/local_parser.h"
#include "data/pybind/pybind_manager.h"

// Position 0 reserved to indicate error
constexpr auto internal_id_start = 1;

/**
 * Internal id which will be assigned to the next prototype added
 */
unsigned int internal_id_new = internal_id_start;

/**
 * Appended to the beginning of each new prototype
 */
std::string directory_prefix;

void jactorio::data::SetDirectoryPrefix(const std::string& name) {
	directory_prefix = name;
}

void jactorio::data::DataRawAdd(const std::string& iname,
                                PrototypeBase* const prototype,
                                const bool add_directory_prefix) {
	const DataCategory data_category = prototype->Category();

	// Use the following format internal name
	// Format __dir__/iname
	std::string formatted_iname;
	{
		std::ostringstream sstr;
		if (add_directory_prefix)
			sstr << "__" << directory_prefix << "__/";

		sstr << iname;
		formatted_iname = sstr.str();
	}

	// Print warning if overriding another name
	// Do not print warning in iname is empty and assign a new unique name
	if (iname.empty()) {
		// Generate a internal name based on the id
		std::ostringstream sstr;
		sstr << "@" << internal_id_new;
		formatted_iname = sstr.str();
	}
	else {
		const auto& category = data_raw[static_cast<uint16_t>(data_category)];
		auto it              = category.find(formatted_iname);
		if (it != category.end()) {
			LOG_MESSAGE_f(warning,
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
		prototype->order = internal_id_new;
	if (prototype->GetLocalizedName().empty())
		prototype->SetLocalizedName(formatted_iname);

	prototype->internalId = internal_id_new++;


	data_raw[static_cast<uint16_t>(data_category)][formatted_iname] = prototype;
	LOG_MESSAGE_f(debug, "Added prototype %d %s", data_category, formatted_iname.c_str());
}

void jactorio::data::LoadData(
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
				LOG_MESSAGE_f(warning, "Directory %s has no or empty data.py file. Ignoring", current_directory.c_str())
				continue;
			}


			SetDirectoryPrefix(directory_name);
			try {
				PyExec(py_file_contents, py_file_path.str());
			}
			catch (DataException& e) {
				LOG_MESSAGE_f(error, "%s", e.what());
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
				LOG_MESSAGE_f(warning,
				              "Directory %s missing local at %s",
				              current_directory.c_str(),
				              cfg_file_path.str().c_str())
				continue;
			}

			auto local_contents = core::ReadFile(cfg_file_path.str());
			LocalParseNoThrow(local_contents, directory_name);
		}

		LOG_MESSAGE_f(info, "Directory %s loaded", current_directory.c_str())
	}

	LOG_MESSAGE(info, "Validating loaded prototypes")
	for (auto& prototype_categories : data_raw) {
		for (auto& pair : prototype_categories) {
			auto& prototype = *pair.second;
			LOG_MESSAGE_f(debug, "Validating prototype %d %s", prototype.internalId, prototype.name.c_str());

			prototype.PostLoad();
			try {
				prototype.PostLoadValidate();
			}
			catch (DataException& e) {
				LOG_MESSAGE_f(error, "Prototype validation failed: '%s'", e.what());
				throw;
			}
			prototype.ValidatedPostLoad();

			LOG_MESSAGE_f(debug, "Validating prototype %d %s Success\n", prototype.internalId, prototype.name.c_str());
		}
	}
}

void jactorio::data::ClearData() {
	// Iterate through both unordered maps and delete all pointers
	for (auto& map : data_raw) {
		// Category unordered maps
		for (auto& category_pair : map) {
			delete category_pair.second;
		}
		map.clear();
	}

	internal_id_new = internal_id_start;
}
