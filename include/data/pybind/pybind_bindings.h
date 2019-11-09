#ifndef DATA_PYBIND_PYBIND_BINDINGS_H
#define DATA_PYBIND_PYBIND_BINDINGS_H

#include <pybind11/stl.h>
#include <pybind11/pybind11.h>

#include "data/data_manager.h"
#include "data/prototype/prototype_base.h"

// All the bindings in bindings/ defined for pybind
// This should only be included by pybind_manager.h

// Types to pass by reference
using data_category = jactorio::data::data_category;
using prototype_base = jactorio::data::Prototype_base;
using data_raw = std::unordered_map<data_category, std::unordered_map<
	                                    std::string, jactorio::data::Prototype_base>>;

PYBIND11_MAKE_OPAQUE(data_raw)



PYBIND11_EMBEDDED_MODULE(jactorio_data, m) {
	// `m` is a `py::module` which is used to bind functions and classes
	py::class_<data_raw>(m, "Prototype_data")
		.def(py::init());

	py::class_<prototype_base>(m, "Prototype")
		.def(py::init())
		.def_readwrite("name", &prototype_base::name)
		.def("load_sprite", &prototype_base::load_sprite);
	
	py::enum_<data_category>(m, "category")
		.value("Tile", data_category::tile);

	
	// Functions defined for the class bindings must be actually within the class in c++
	m.def("get", [](const data_category category, const std::string iname) {
		// Cannot give pointers back!
		return *jactorio::data::data_manager::data_raw_get(category, iname);
	});

	m.def("add", [](const data_category category, const std::string iname,
	                const prototype_base prototype) {
		
		jactorio::data::data_manager::data_raw_add(category, iname, prototype);
	});
	
}


#endif // DATA_PYBIND_PYBIND_BINDINGS_H
