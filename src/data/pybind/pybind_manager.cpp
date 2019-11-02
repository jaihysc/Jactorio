#include "data/pybind/pybind_manager.h"

#include <pybind11/embed.h>

#include "core/logger.h"

namespace logger = jactorio::core::logger;
namespace py = pybind11;

// Bindings to expose to python
#include "data/pybind/bindings/data.h"

void jactorio::data::pybind_manager::eval(const std::string& python_str) {
	try {
		py::eval_file(python_str);
	}
		// Catches python execution errors
	catch (py::error_already_set& err) {
		log_message(logger::error, "Python interpreter", err.what());
	}

}

void jactorio::data::pybind_manager::py_interpreter_init() {
	py::initialize_interpreter();
	log_message(logger::debug, "Python interpreter", "Interpreter initialized");
}

void jactorio::data::pybind_manager::py_interpreter_terminate() {
	py::finalize_interpreter();
	log_message(logger::debug, "Python interpreter", "Interpreter terminated");
}
