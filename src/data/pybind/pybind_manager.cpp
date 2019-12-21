#include "data/pybind/pybind_manager.h"

#include <pybind11/embed.h>

#include "core/logger.h"

namespace py = pybind11;

// Bindings to expose to python
#include "data/pybind/pybind_bindings.h"

py::object py_stdout;
py::object py_stderr;
py::object py_stdout_buffer;
py::object py_stderr_buffer;

std::string error_message;

const std::string& jactorio::data::pybind_manager::get_last_error_message() {
	return error_message;
}

int jactorio::data::pybind_manager::exec(const std::string& python_str, const std::string& file_name) {
	try {
		py::exec(python_str);

		// Seek any output from python sys.stdout
		py_stdout_buffer.attr("seek")(0);
		py_stderr_buffer.attr("seek")(0);
		const std::string std_out = py::str(py_stdout_buffer.attr("read")());
		const std::string std_err = py::str(py_stderr_buffer.attr("read")());

		if (!std_out.empty()) {
			LOG_MESSAGE_f(info, "%s %s", file_name.c_str(), std_out.c_str())
		}
		if (!std_err.empty()) {
			LOG_MESSAGE_f(error, "%s - %s", file_name.c_str(), std_err.c_str())
		}
		
		return 0;
	}
		// Catches python execution errors
	catch (py::error_already_set& err) {
		error_message = err.what();
		// log_message(logger::error, "Python interpreter", err_msg);
		return 1;
	}

}

void jactorio::data::pybind_manager::py_interpreter_init() {
	py::initialize_interpreter();

	// Used to redirect python sys.stdout
	const auto sysm = py::module::import("sys");
	py_stdout = sysm.attr("stdout");
	py_stderr = sysm.attr("stderr");

	// Other file like object can be used here as well, such as objects created by pybind11
	const auto stringio = py::module::import("io").attr("StringIO");
	py_stdout_buffer = stringio();
	py_stderr_buffer = stringio();
	
	sysm.attr("stdout") = py_stdout_buffer;
	sysm.attr("stderr") = py_stderr_buffer;

	LOG_MESSAGE(debug, "Python interpreter initialized")
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

	LOG_MESSAGE(debug, "Python interpreter terminated")
}
