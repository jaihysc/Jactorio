#ifndef DATA_PYBIND_PYBIND_MANAGER_H
#define DATA_PYBIND_PYBIND_MANAGER_H

#include <string>

/**
 * The python interpreter is NOT thread safe, call py_interpreter_init for each thread it needs to be used in
 */
namespace jactorio::data::pybind_manager
{
	/**
	 * Evaluates string of python
	 * @param python_str Python string to interpret
	 * @param file_name if specified, used to display logging information from python sys.stdout
	 * @exception Data_exception if error occurred
	 */
	int exec(const std::string& python_str, const std::string& file_name = "");

	void py_interpreter_init();
	void py_interpreter_terminate();
}

#endif // DATA_PYBIND_PYBIND_MANAGER_H
