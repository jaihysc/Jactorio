#include "data/pybind/pybind_manager.h"

#include <sstream>
#include <pybind11/embed.h>

#include "core/logger.h"
#include "core/filesystem.h"
#include "data/data_exception.h"

namespace py = pybind11;

// Bindings to expose to python
#include "data/pybind/pybind_bindings.h"

py::object py_stdout;
py::object py_stderr;
py::object py_stdout_buffer;
py::object py_stderr_buffer;

int jactorio::data::pybind_manager::exec(const std::string& python_str, const std::string& file_name) {
	try {
		// Redirect python sys.stdout to c++
		const auto string_io = py::module::import("io").attr("StringIO");
		py_stdout_buffer = string_io();
		py_stderr_buffer = string_io();

		const auto sys = py::module::import("sys");
		sys.attr("stdout") = py_stdout_buffer;
		sys.attr("stderr") = py_stderr_buffer;

		
		py::exec(python_str);


		// Seek any output from python sys.stdout
		py_stdout_buffer.attr("seek")(0);
		py_stderr_buffer.attr("seek")(0);
		const std::string std_out = py::str(py_stdout_buffer.attr("read")());
		const std::string std_err = py::str(py_stderr_buffer.attr("read")());

		py_stdout_buffer.release();
		py_stderr_buffer.release();
		
		if (!std_out.empty()) {
			LOG_MESSAGE_f(info, "Python: %s %s", file_name.c_str(), std_out.c_str())
		}
		if (!std_err.empty()) {
			LOG_MESSAGE_f(error, "Python: %s - %s", file_name.c_str(), std_err.c_str())
		}
		
		return 0;
	}
		// Catches python execution errors
	catch (py::error_already_set& err) {
		std::ostringstream oss;
		oss << file_name << "\n" << err.what();
		throw Data_exception(oss.str());
	}

}

void jactorio::data::pybind_manager::py_interpreter_init() {
	py::initialize_interpreter();

	// Used to redirect python sys.stdout
	const auto sysm = py::module::import("sys");
	{
		// Clear all existing import paths to avoid conflicts
		sysm.attr("path").attr("clear")();
		
		// Include the data_manager::data_folder/ as a python search path to shorten imports
		std::stringstream s;
		s << core::filesystem::get_executing_directory() << "/"
			<< data_manager::data_folder << "/";
		sysm.attr("path").attr("append")(s.str());
	}

	py_stdout = sysm.attr("stdout");
	py_stderr = sysm.attr("stderr");

	LOG_MESSAGE(info, "Python interpreter initialized")
}

void jactorio::data::pybind_manager::py_interpreter_terminate() {
	// Redirect python sys.stdout
	const auto sysm = py::module::import("sys");
	sysm.attr("stdout") = py_stdout;
	sysm.attr("stderr") = py_stderr;

	py_stdout.release();
	py_stderr.release();
	py_stdout_buffer.release();
	py_stderr_buffer.release();

	// Remove all py::objects before deleting the interpreter
	py::finalize_interpreter();

	LOG_MESSAGE(info, "Python interpreter terminated")
}
