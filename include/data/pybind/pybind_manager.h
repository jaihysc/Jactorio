// 
// pybind_manager.h
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 11/09/2019
// Last modified: 03/14/2020
// 

#ifndef JACTORIO_INCLUDE_DATA_PYBIND_PYBIND_MANAGER_H
#define JACTORIO_INCLUDE_DATA_PYBIND_PYBIND_MANAGER_H
#pragma once

#include <string>

namespace jactorio::data::pybind_manager
{
	/**
	 * Evaluates string of python
	 * @param python_str Python string to interpret
	 * @param file_name if specified, used to display logging information from python sys.stdout
	 * @exception Data_exception if error occurred
	 */
	int exec(const std::string& python_str, const std::string& file_name = "");

	///
	/// \remarks The python interpreter is NOT thread safe, call py_interpreter_init for each thread it needs to be used in
	void py_interpreter_init();
	void py_interpreter_terminate();
}

#endif //JACTORIO_INCLUDE_DATA_PYBIND_PYBIND_MANAGER_H
