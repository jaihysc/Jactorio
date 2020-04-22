// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/22/2019

#ifndef JACTORIO_INCLUDE_CORE_FILESYSTEM_H
#define JACTORIO_INCLUDE_CORE_FILESYSTEM_H
#pragma once

#include <string>

namespace jactorio::core
{
	std::string get_executing_directory();

	///
	/// \brief Replaces ~ at the beginning of the path with the executing directory <br>
	/// \remark If there is no ~ at the beginning of the path, nothing happens
	std::string resolve_path(const std::string& path);

	///
	/// \remark Cuts off file names (some/where/executing.exe) becomes (some/where)
	void set_executing_directory(const std::string& directory);

	///
	/// \brief Reads file from path as string
	/// \return Empty string if path is invalid
	std::string read_file_as_str(const std::string& path);
}

#endif //JACTORIO_INCLUDE_CORE_FILESYSTEM_H
