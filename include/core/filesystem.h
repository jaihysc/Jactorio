#ifndef CORE_FILE_SYSTEM_H
#define CORE_FILE_SYSTEM_H

#include <string>

namespace jactorio::core::filesystem
{
	std::string get_executing_directory();

	/**
	 * Replaces ~ at the beginning of the path with the executing directory <br>
	 * If there is no ~ at the beginning of the path, nothing happens
	 */
	std::string resolve_path(const std::string& path);

	/**
	 * Cuts off file names (some/where/executing.exe) becomes (some/where)
	 */
	void set_executing_directory(const std::string& executing_directory);

	/**
	 * Reads file from path as string <br>
	 * Returns empty string if path is invalid
	 */
	std::string read_file_as_str(const std::string& path);
}

#endif // CORE_FILE_SYSTEM_H
