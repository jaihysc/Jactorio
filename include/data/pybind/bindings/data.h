#ifndef DATA_PYBIND_BINDINGS_DATA_H
#define DATA_PYBIND_BINDINGS_DATA_H

#include <pybind11/embed.h>

PYBIND11_EMBEDDED_MODULE(jactorio_data, m) {
	// `m` is a `py::module` which is used to bind functions and classes
	m.def("add", [](int i, int j) {
		return i + j;
	});
}

#endif // DATA_PYBIND_BINDINGS_DATA_H
