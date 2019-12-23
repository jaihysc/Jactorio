#include "data/data_manager.h"

#include <filesystem>
#include <sstream>

#include "core/filesystem.h"
#include "core/logger.h"
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

void jactorio::data::data_manager::set_directory_prefix(const std::string& name) {
	directory_prefix = name;
}

void jactorio::data::data_manager::data_raw_add(const data_category data_category, const std::string& iname,
                                                Prototype_base* const prototype, const bool add_directory_prefix) {
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
	const auto& category = data_raw[data_category];
	if (category.find(formatted_iname) != category.end()) {
		LOG_MESSAGE_f(warning, "Name \"%s\" type %d overrides previous declaration",
		              formatted_iname.c_str(), static_cast<int>(data_category));
	}
	
	// Enforce prototype rules...
	// data_category should be the same as category
	// No order specified, use internal id as order
	prototype->name = formatted_iname;

	if (prototype->category != data_category)
		prototype->category = data_category;
	if (prototype->order == 0)
		prototype->order = internal_id_new;
	if (prototype->localized_name.empty())
		prototype->localized_name = formatted_iname;

	prototype->internal_id = internal_id_new++;


	data_raw[data_category][formatted_iname] = prototype;
}

int jactorio::data::data_manager::load_data(
	const std::string& data_folder_path) {
	// Get all sub-folders in ~/data/
	// Read data.cfg files within each sub-folder
	// Load extracted data into loaded_data

	for (const auto& entry : std::filesystem::directory_iterator(
		     data_folder_path)) {
		const std::string directory_name = entry.path().filename().u8string();

		std::stringstream ss;
		ss << data_folder_path << "/" << directory_name << "/data.py";

		const std::string py_file_contents = core::filesystem::
			read_file_as_str(ss.str());

		// data.cfg file does not exist
		if (py_file_contents.empty()) {
			LOG_MESSAGE_f(warning, "Directory %s/%s has no or empty data.py file. Ignoring",
			              data_folder_path.c_str(),
			              directory_name.c_str())
			continue;
		}


		set_directory_prefix(directory_name);
		if (pybind_manager::exec(py_file_contents, ss.str()) != 0) {
			// Error occurred
			LOG_MESSAGE_f(error, "%s", pybind_manager::get_last_error_message().c_str());
			return 1;
		}

		LOG_MESSAGE_f(info, "Directory %s/%s loaded",
		              data_folder_path.c_str(),
		              directory_name.c_str())
	}

	return 0;
}

void jactorio::data::data_manager::clear_data() {
	// Iterate through both unordered maps and delete all pointers
	for (auto& pair : data_raw) {
		// Category unordered maps
		for (auto& category_pair : pair.second) {
			delete category_pair.second;
		}
	}
	
	data_raw.clear();
	internal_id_new = internal_id_start;
}
