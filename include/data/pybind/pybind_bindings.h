#ifndef DATA_PYBIND_PYBIND_BINDINGS_H
#define DATA_PYBIND_PYBIND_BINDINGS_H

#include <pybind11/stl.h>
#include <pybind11/pybind11.h>

#include "data/data_manager.h"
#include "data/prototype/prototype_base.h"
#include "data/prototype/sprite.h"
#include "data/prototype/tile.h"

// All the bindings in bindings/ defined for pybind
// This should only be included by pybind_manager.h

using data_raw = std::unordered_map<jactorio::data::data_category, std::unordered_map<
	                                    std::string, jactorio::data::Prototype_base>>;

// Types to pass by reference
PYBIND11_MAKE_OPAQUE(data_raw)



PYBIND11_EMBEDDED_MODULE(jactorio_data, m) {
	using namespace jactorio::data;
	
	// Functions defined for the class bindings must be actually within the class in c++

	// Prototype classes
	py::class_<Prototype_base>(m, "Prototype")
		.def(py::init())
		.def_readwrite("name", &Prototype_base::name)
		.def_readwrite("category", &Prototype_base::category);

	py::class_<Sprite, Prototype_base>(m, "Sprite")
		.def(py::init())
		.def(py::init<const std::string&>())
		.def_readwrite("sprite_image", &Sprite::sprite_image)
		.def("load_sprite", &Sprite::load_sprite);
	
	py::class_<Tile, Prototype_base>(m, "Tile")
		.def(py::init())
		.def(py::init<Sprite*>())
		.def_readwrite("sprite", &Tile::sprite_ptr);

	
	// ############################################################
	// Data_raw + get/set

	py::enum_<data_category>(m, "category")
		.value("Tile", data_category::tile)
		.value("Sprite", data_category::sprite)
		.value("Noise_layer", data_category::noise_layer)
		.value("Sound", data_category::sound);
	
	py::class_<data_raw>(m, "Prototype_data")
		.def(py::init());
	
	m.def("get", [](const data_category category, const std::string iname) {
		// Cannot give pointers back!
		return *data_manager::data_raw_get<Prototype_base>(category, iname);
	});

	m.def("add", [](const data_category category, const std::string iname,
	                const py::object prototype) {
		py::object* const object = new py::object{};
		*object = prototype;
		
		data_manager::data_raw_add(category, iname, 
		                           py::cast<Prototype_base*>(*object));
	});

	// ############################################################

}


#endif // DATA_PYBIND_PYBIND_BINDINGS_H
