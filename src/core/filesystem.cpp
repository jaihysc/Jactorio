// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/22/2019

#include <fstream>
#include <sstream>
#include <string>

#include "core/filesystem.h"

#include "core/logger.h"

std::string executing_directory;

std::string jactorio::core::filesystem::get_executing_directory() {
	return executing_directory;
}

void jactorio::core::filesystem::set_executing_directory(const std::string& directory) {
	std::string path = directory;

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

	LOG_MESSAGE_f(info, "Set executing directory: %s", path.c_str());

	executing_directory = path;
}

std::string jactorio::core::filesystem::resolve_path(const std::string& path) {
	if (!path.empty() && path[0] == '~') {
		std::string path_copy = path;

		std::stringstream sstr;
		sstr << executing_directory << path_copy.erase(0, 1);
		return sstr.str();
	}

	return path;
}


std::string jactorio::core::filesystem::read_file_as_str(const std::string& path) {
	const std::ifstream in(path);

	std::stringstream sstr;
	sstr << in.rdbuf();
	return sstr.str();
}
