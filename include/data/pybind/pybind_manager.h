#ifndef DATA_PYBIND_PYBIND_MANAGER_H
#define DATA_PYBIND_PYBIND_MANAGER_H

#include <string>

namespace jactorio::data::pybind_manager
{
	void eval(const std::string& python_str);

	void py_interpreter_init();
	void py_interpreter_terminate();
}

#endif // DATA_PYBIND_PYBIND_MANAGER_H
