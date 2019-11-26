#include "data/data_manager.h"

#include <filesystem>
#include <sstream>

#include "core/filesystem.h"
#include "core/logger.h"
#include "data/pybind/pybind_manager.h"

void jactorio::data::data_manager::data_raw_add(const data_category data_category, const std::string& iname,
                                                Prototype_base* const prototype) {
	// Enforce prototype name must be the same as iname
	// data_category should be the same as category
	if (prototype->name != iname) {
		prototype->name = iname;
	}
	if (prototype->category != data_category)
		prototype->category = data_category;
	
	data_raw[data_category][iname] = prototype;
}


void jactorio::data::data_manager::load_data(
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


		std::string result = pybind_manager::exec(py_file_contents, directory_name);
		if (!result.empty()) {
			// Error occurred
			LOG_MESSAGE_f(error, "%s %s", ss.str().c_str(), result.c_str())
			continue;
		}

		LOG_MESSAGE_f(info, "Directory %s/%s loaded",
		              data_folder_path.c_str(),
		              directory_name.c_str())
	}
}

void jactorio::data::data_manager::clear_data() {
	data_raw.clear();
}
