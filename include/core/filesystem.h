// 
// filesystem.h
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 10/22/2019
// Last modified: 03/14/2020
// 

#ifndef JACTORIO_INCLUDE_CORE_FILESYSTEM_H
#define JACTORIO_INCLUDE_CORE_FILESYSTEM_H
#pragma once

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

#endif //JACTORIO_INCLUDE_CORE_FILESYSTEM_H
