// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_DATA_PYBIND_PYBIND_MANAGER_H
#define JACTORIO_INCLUDE_DATA_PYBIND_PYBIND_MANAGER_H
#pragma once

#include <string>

namespace jactorio::data
{
    ///
    /// \brief Evaluates string of python
    /// \param python_str Python string to interpret
    /// \param file_name if specified, used to display logging information from python sys.stdout
    /// \exception Data_exception if error occurred
    int PyExec(const std::string& python_str, const std::string& file_name = "");

    ///
    /// \remarks The python interpreter is NOT thread safe, call py_interpreter_init for each thread it needs to be used
    /// in
    void PyInterpreterInit();
    void PyInterpreterTerminate();
} // namespace jactorio::data

#endif // JACTORIO_INCLUDE_DATA_PYBIND_PYBIND_MANAGER_H
