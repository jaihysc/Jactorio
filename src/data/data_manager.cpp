#include "data/data_manager.h"

#include <filesystem>
#include <sstream>
#include <vector>

#include "core/filesystem.h"
#include "core/logger.h"
#include "data/pybind/pybind_manager.h"

// Example: data_raw[image]["grass-1"] -> Prototype
std::unordered_map<jactorio::data::data_category, std::unordered_map<
	                   std::string, jactorio::data::Prototype_base>> data_raw;


jactorio::data::Prototype_base* jactorio::data::data_manager::data_raw_get(const data_category data_category,
                                                                           const std::string& iname) {
	auto category = &data_raw[data_category];
	if (category->find(iname) == category->end())
		return nullptr;

	// Address of prototype item - category is a pointer
	return &(*category)[iname];
}

void jactorio::data::data_manager::data_raw_add(const data_category data_type, const std::string& iname,
                                                const Prototype_base& prototype) {
	// Enforce prototype name must be the same as iname
	if (prototype.name != iname) {
		auto local_proto = prototype;
		local_proto.name = iname;

		data_raw[data_type][iname] = local_proto;
	}
	else
		data_raw[data_type][iname] = prototype;
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
			std::stringstream log_ss;
			log_ss << "Directory " << data_folder_path << "/" << directory_name
				<< " has empty or no data.py file. Ignoring.";
			log_message(core::logger::warning,
			            "Data manager",
			            log_ss.str());

			continue;
		}


		std::string result = pybind_manager::exec(py_file_contents, directory_name);
		if (!result.empty()) {
			// Error occurred
			log_message(core::logger::error, "Data manager", result);
			continue;
		}

		std::stringstream log_ss;
		log_ss << data_folder_path << "/" << directory_name <<
			" loaded successfully";
		log_message(core::logger::info,
		            "Data manager", log_ss.str());
	}
}

std::string jactorio::data::data_manager::get_path(
	const data_category type, const std::string& iname) {
	// Ensure type and id exists
	if (!data_raw.count(type))
		return "!";
	if (!data_raw[type].count(iname))
		return "!";

	// return data_raw[type][iname];
	return "";
}

std::vector<jactorio::data::Prototype_base> jactorio::data::data_manager::get_all_data(
	const data_category type) {
	
	auto m = data_raw[type];
	
	std::vector<Prototype_base> paths;
	paths.reserve(m.size());
	
	for (auto& it : m) {
		paths.push_back(it.second);
	}
	
	return paths;
}
