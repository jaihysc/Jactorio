#include <string>
#include <fstream>
#include <sstream>

#include "core/file_system.h"

// Reads file from path as string
// Returns NULL if path is invalid
std::string read_file_as_str(const std::string& path) {
	const std::ifstream in(path);
	
	std::stringstream sstr;
	sstr << in.rdbuf();
	return sstr.str();
}
