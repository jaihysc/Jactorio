#include "data/data_manager.h"

#include <filesystem>
#include <sstream>
#include <vector>

#include "core/filesystem.h"
#include "core/logger.h"
#include "data/startup/data_cfg_parser.h"

// Accessed via data access methods
// std::string type -> Everything of specified type (E.g image, audio)
// >> std::string id - > std::string path to item

// Example loaded_data[image]["grass-1"] -> std::string path
std::unordered_map<jactorio::data::data_type, std::unordered_map<
	                   std::string, std::string>> loaded_data;

void jactorio::data::data_manager::load_data(
	const std::string& data_folder_path) {
	// Get all sub-folders in ~/data/
	// Read data.cfg files within each sub-folder
	// Load extracted data into loaded_data

	for (const auto& entry : std::filesystem::directory_iterator(
		     data_folder_path)) {
		const std::string directory_name = entry.path().filename().u8string();

		std::stringstream ss;
		ss << data_folder_path << "/" << directory_name << "/data.cfg";

		const std::string cfg_file_contents = core::filesystem::
			read_file_as_str(ss.str());

		// data.cfg file does not exist
		if (cfg_file_contents.empty()) {
			std::stringstream log_ss;
			log_ss << "Directory " << data_folder_path << "/" << directory_name
				<< " has no data.cfg file. Ignoring.";
			log_message(core::logger::warning,
			            "Data manager",
			            log_ss.str());

			continue;
		}


		{
			Data_cfg_parser parser{};
			Cfg_data data = parser.parse(
				cfg_file_contents, directory_name);

			// Add data to loaded_data_
			for (unsigned int i = 0; i < data.count; ++i) {
				Cfg_data_element& element = data.elements[i];

				loaded_data[element.type][element.id] = element.path;
			}

			delete[] data.elements;


			std::stringstream log_ss;
			log_ss << data_folder_path << "/" << directory_name <<
				" loaded successfully";
			log_message(core::logger::info,
			            "Data manager", log_ss.str());
		}
	}
}

std::string jactorio::data::data_manager::get_path(
	const data_type type, const std::string& iname) {
	// Ensure type and id exists
	if (!loaded_data.count(type))
		return "!";
	if (!loaded_data[type].count(iname))
		return "!";

	return loaded_data[type][iname];
}

std::string jactorio::data::data_manager::get_iname(const data_type type, const std::string& path) {
	auto inner_pairs_map = loaded_data[type];

	for (auto& it : inner_pairs_map) {
		if (it.second == path)
			return it.first;
	}
	
	return "!";
}

std::vector<std::string> jactorio::data::data_manager::get_all_data(const data_type type) {
	auto m = loaded_data[type];
	
	std::vector<std::string> paths;
	paths.reserve(m.size());
	
	for (auto& it : m) {
		paths.push_back(it.second);
	}
	
	return paths;
}
