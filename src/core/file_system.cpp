#include <string>
#include <fstream>
#include <sstream>

#include "core/file_system.h"

#include "core/logger.h"

std::string jactorio::core::File_system::executing_directory_;

void jactorio::core::File_system::set_executing_directory(const std::string& executing_directory) {
	std::string path = executing_directory;
	
	// Convert backwards slashes to forwards if on windows
#ifdef WIN32
	for (char& i : path) {
		if (i == '\\')
			i = '/';
	}
#endif

	{
		// Get first /
		int i = path.size();
		while (i >= 0 && path[i] != '/')
			--i;

		if (i < 0)  // In case / does not exist
			i = 0;
		
		// Erase everything prior to first /
		path.erase(i, path.size() - i);	
	}

	std::stringstream sstr;
	sstr << "Set executing directory: " << path;
	log_message(logger::info, "File system", sstr.str());
	
	executing_directory_ = path;
}

std::string jactorio::core::File_system::resolve_path(const std::string& path) {
	if (!path.empty() && path[0] == '~') {
		std::string path_copy = path;
		
		std::stringstream sstr;
		sstr << executing_directory_ << path_copy.erase(0, 1);
		return sstr.str();
	}

	return path;
}


std::string jactorio::core::File_system::read_file_as_str(const std::string& path) {
	const std::ifstream in(path);
	
	std::stringstream sstr;
	sstr << in.rdbuf();
	return sstr.str();
}
