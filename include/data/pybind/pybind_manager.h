#ifndef DATA_PYBIND_PYBIND_MANAGER_H
#define DATA_PYBIND_PYBIND_MANAGER_H

#include <string>

/**
 * The python interpreter is NOT thread safe, call py_interpreter_init for each thread it needs to be used in
 */
namespace jactorio::data::pybind_manager
{
	const std::string& get_last_error_message();
	/**
	 * Evaluates string of python
	 * @param python_str Python string to interpret
	 * @param file_name if specified, used to display logging information from python sys.stdout
	 * @return non-zero if errors occurred. error message stored in get_error_message()
	 */
	int exec(const std::string& python_str, const std::string& file_name = "");

	void py_interpreter_init();
	void py_interpreter_terminate();
}

#endif // DATA_PYBIND_PYBIND_MANAGER_H
