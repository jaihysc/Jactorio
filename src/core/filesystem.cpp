// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/22/2019

#include "core/filesystem.h"

#include <fstream>
#include <sstream>
#include <string>

#include "jactorio.h"

std::string executing_directory;

std::string jactorio::core::GetExecutingDirectory() {
	return executing_directory;
}

void jactorio::core::SetExecutingDirectory(const std::string& directory) {
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
		int64_t i = path.size();
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

std::string jactorio::core::ResolvePath(const std::string& path) {
	// Catch old usages of ResolvePath
	// if (!path.empty()) {
	// 	assert(path[0] != '~');
	// }

	std::string str = executing_directory;
	return str.append("/").append(path);
}


std::string jactorio::core::ReadFile(const std::string& path) {
	const std::ifstream in(path);

	std::stringstream sstr;
	sstr << in.rdbuf();
	return sstr.str();
}
