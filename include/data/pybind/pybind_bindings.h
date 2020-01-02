#ifndef DATA_PYBIND_PYBIND_BINDINGS_H
#define DATA_PYBIND_PYBIND_BINDINGS_H

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "data/data_manager.h"
#include "data/prototype/entity/container_entity.h"
#include "data/prototype/entity/entity.h"
#include "data/prototype/entity/health_entity.h"
#include "data/prototype/item/item.h"
#include "data/prototype/noise_layer.h"
#include "data/prototype/prototype_base.h"
#include "data/prototype/sprite.h"
#include "data/prototype/tile/enemy_tile.h"
#include "data/prototype/tile/resource_tile.h"
#include "data/prototype/tile/tile.h"

// All the bindings in bindings/ defined for pybind
// This should only be included by pybind_manager.h

using data_raw = std::unordered_map<jactorio::data::data_category, std::unordered_map<
	                                    std::string, jactorio::data::Prototype_base>>;

// Types to pass by reference
PYBIND11_MAKE_OPAQUE(data_raw)


PYBIND11_EMBEDDED_MODULE(jactorioData, m) {
	using namespace jactorio::data;

	// Functions defined for the class bindings must be actually within the class in c++

	// Prototype classes
	py::class_<Prototype_base>(m, "PrototypeBase")
		// .def_readwrite("name", &Prototype_base::name)
		.def_readwrite("category", &Prototype_base::category)
		.def_readwrite("order", &Prototype_base::order);

	py::class_<Sprite, Prototype_base>(m, "Sprite")
		.def(py::init())
		.def(py::init<const std::string&>())
		.def(py::init<const std::string&, Sprite::sprite_group>())
		.def_readwrite("group", &Sprite::group)
		.def("load", &Sprite::load_image);
	py::enum_<Sprite::sprite_group>(m, "spriteGroup")
		.value("Terrain", Sprite::sprite_group::terrain)
		.value("Gui", Sprite::sprite_group::gui);


	py::class_<Item, Prototype_base>(m, "Item")
		.def_readwrite("sprite", &Item::sprite)
		.def_readwrite("stackSize", &Item::stack_size);


	py::class_<Tile, Prototype_base>(m, "Tile")
		.def_readwrite("isWater", &Tile::is_water)
		.def_readwrite("sprite", &Tile::sprite_ptr);

	py::class_<Resource_tile, Tile>(m, "ResourceTile");

	py::class_<Noise_layer, Prototype_base>(m, "NoiseLayer")
		// Perlin noise properties
		.def_readwrite("octaveCount", &Noise_layer::octave_count)
		.def_readwrite("frequency", &Noise_layer::frequency)
		.def_readwrite("persistence", &Noise_layer::persistence)

		.def_readwrite("tileDataCategory", &Noise_layer::tile_data_category)
		.def_readwrite("normalize", &Noise_layer::normalize_val)
		.def("getStartVal", &Noise_layer::get_start_val)
		.def("startVal", &Noise_layer::set_start_val)
		.def("addTile", &Noise_layer::add_tile)
		.def("getTile", &Noise_layer::get_tile);


	// Entity
	py::class_<Entity, Prototype_base>(m, "Entity")
		.def_readwrite("item", &Entity::item)
		.def_readwrite("rotatable", &Entity::rotatable)
		.def_readwrite("tileWidth", &Entity::tile_width)
		.def_readwrite("tileHeight", &Entity::tile_height);

	py::class_<Health_entity, Entity>(m, "HealthEntity")
		.def_readwrite("maxHealth", &Health_entity::max_health);

	py::class_<Container_entity, Health_entity>(m, "ContainerEntity")
		.def_readwrite("sprite", &Container_entity::sprite)
		.def_readwrite("inventorySize", &Container_entity::inventory_size);

	// ############################################################
	// Data_raw + get/set

	py::enum_<data_category>(m, "category")
		.value("Tile", data_category::tile)
		.value("ResourceTile", data_category::resource_tile)
		.value("EnemyTile", data_category::enemy_tile)
		.value("Sprite", data_category::sprite)
		.value("NoiseLayer", data_category::noise_layer)
		.value("Sound", data_category::sound)
		.value("Item", data_category::item)

		.value("Entity", data_category::entity)
		.value("HealthEntity", data_category::health_entity)
		.value("ContainerEntity", data_category::container_entity);

	py::class_<data_raw>(m, "Prototype_data")
		.def(py::init());

	m.def("get", [](const data_category category, const std::string& iname) {
		// Cannot give pointers back!
		return *data_manager::data_raw_get<Prototype_base>(category, iname);
	});


#define PROTOTYPE_CATEGORY(category, class_) case data_category::category: prototype = new (class_); break;
	/**
	 * Call add from python to retrieve a new pointer to the prototype <br>
	 * Prototypes cannot be made in Python as they need to be copied back to c++
	 */
	m.def("add", [](const data_category category, const std::string& iname = "") -> Prototype_base* {
		assert(category != data_category::none);
		
		Prototype_base* prototype = nullptr;
		
		switch (category) {
			PROTOTYPE_CATEGORY(tile, Tile);
			PROTOTYPE_CATEGORY(resource_tile, Resource_tile);
			PROTOTYPE_CATEGORY(enemy_tile, Enemy_tile);
			PROTOTYPE_CATEGORY(sprite, Sprite);
			PROTOTYPE_CATEGORY(noise_layer, Noise_layer);
			// PROTOTYPE_CATEGORY(sound, Sound);
			PROTOTYPE_CATEGORY(item, Item);
			PROTOTYPE_CATEGORY(entity, Entity);
			PROTOTYPE_CATEGORY(health_entity, Health_entity);
			PROTOTYPE_CATEGORY(container_entity, Container_entity);
		
		default:
			assert(false);  // Missing case for category
			break;
		}
		
		data_manager::data_raw_add(category, iname, prototype, true);
		return prototype;
	}, py::arg("category"), py::arg("iname") = "", pybind11::return_value_policy::reference);
#undef PROTOTYPE_CATEGORY
	
	// ############################################################

}


#endif // DATA_PYBIND_PYBIND_BINDINGS_H
