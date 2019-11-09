#ifndef DATA_PYBIND_PYBIND_MANAGER_H
#define DATA_PYBIND_PYBIND_MANAGER_H

#include <string>

namespace jactorio::data::pybind_manager
{
	/**
	 * Evaluates string of python
	 * @param python_str Python string to interpret
	 * @param file_name if specified, used to display logging information from python sys.stdout
	 * @return empty string if no errors occurred, if errors occurred-string is error message
	 */
	std::string exec(const std::string& python_str, const std::string& file_name = "");

	void py_interpreter_init();
	void py_interpreter_terminate();
}

#endif // DATA_PYBIND_PYBIND_MANAGER_H
