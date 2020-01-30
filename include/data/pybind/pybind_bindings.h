#ifndef DATA_PYBIND_PYBIND_BINDINGS_H
#define DATA_PYBIND_PYBIND_BINDINGS_H

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

#include "data/data_manager.h"
#include "data/prototype/entity/container_entity.h"
#include "data/prototype/entity/entity.h"
#include "data/prototype/entity/health_entity.h"
#include "data/prototype/item/item.h"
#include "data/prototype/noise_layer.h"
#include "data/prototype/prototype_base.h"
#include "data/prototype/sprite.h"
#include "data/prototype/tile/tile.h"
#include "data/prototype/item/recipe_group.h"
#include "data/prototype/item/recipe_category.h"
#include "data/prototype/entity/resource_entity.h"
#include "data/prototype/entity/enemy_entity.h"

// All the bindings in bindings/ defined for pybind
// This should only be included by pybind_manager.h

using data_raw = std::unordered_map<jactorio::data::data_category, std::unordered_map<
	                                    std::string, jactorio::data::Prototype_base*>>;

// Generates a function for constructing a data class
// Example: >Sprite<, inherits >Prototype_base< in category data_category::sprite
// 
// A python object with name _Sprite will be generated to avoid ambiguity
// To construct sprite, a constructor with the name Sprite(...) is available

#define PYBIND_DATA_CLASS(cpp_class_, inherits_, category_)\
	m.def(#cpp_class_, [](const std::string& iname = "") {\
		auto* prototype = new (cpp_class_);\
		data_manager::data_raw_add((jactorio::data::data_category::category_), iname, prototype, true);\
		return prototype;\
	}, py::arg("iname") = "", pybind11::return_value_policy::reference);\
	py::class_<cpp_class_, inherits_>(m, "_" #cpp_class_)

// Python property gets a separate specified name, typically for class names with underscores
#define PYBIND_DATA_CLASS_SEPARATE(cpp_class_, py_name_, inherits_, category_)\
	m.def(#py_name_, [](const std::string& iname = "") {\
		auto* prototype = new (cpp_class_);\
		data_manager::data_raw_add((jactorio::data::data_category::category_), iname, prototype, true);\
		return prototype;\
	}, py::arg("iname") = "", pybind11::return_value_policy::reference);\
	py::class_<cpp_class_, inherits_>(m, "_" #py_name_)

// Macros below generates a self returning setter and the actual variable
// For standard class members, a setter exists: set_NAME_OF_MEMBER
//		This becomes NAME_OF_MEMBER in Python
// To read standard class members, the member NAME_OF_MEMBER is available as _NAME_OF_MEMBER
// (note the leading underscore)

// If a class member utilizes a non-standard settler, a getter must be implemented,
// reading and writing in python becomes:
// 
// NAME_OF_MEMBER to set
// get_NAME_OF_MEMBER to get

/*
	PYBIND_PROP(Prototype_base, category)

	vvv

	.def("category", &Prototype_base::set_category)
	.def_readwrite("_""category", &Prototype_base::category)
*/

// Python name is he same as the cpp name (commonly single word members)
#define PYBIND_PROP(class_, name)\
	.def(#name, &class_::set_##name, pybind11::return_value_policy::reference)\
	.def_readwrite("_" #name, &class_::name)

// If the python name is different from the c++ name
#define PYBIND_PROP_SEPARATE(class_, py_name, cpp_name)\
	.def(#py_name, &class_::set_##cpp_name, pybind11::return_value_policy::reference)\
	.def_readwrite("_" #py_name, &class_::cpp_name)


// If the member utilizes a getter and settler
#define PYBIND_PROP_GET_SET(class_, py_name, cpp_name)\
	.def(#py_name, &class_::set_##cpp_name, pybind11::return_value_policy::reference)\
	.def("get_" #py_name, &class_::get_##cpp_name, pybind11::return_value_policy::reference)


PYBIND11_EMBEDDED_MODULE(jactorioData, m) {
	using namespace jactorio::data;
	
	// Prototype classes
	py::class_<Prototype_base>(m, "PrototypeBase")
		.def("name", &Prototype_base::set_name)
		PYBIND_PROP(Prototype_base, category)
		PYBIND_PROP(Prototype_base, order);

	PYBIND_DATA_CLASS(Sprite, Prototype_base, sprite)
		PYBIND_PROP(Sprite, group)
		.def("load", &Sprite::load_image);
	py::enum_<Sprite::sprite_group>(m, "spriteGroup")
		.value("Terrain", Sprite::sprite_group::terrain)
		.value("Gui", Sprite::sprite_group::gui);

	PYBIND_DATA_CLASS(Item, Prototype_base, item)
		PYBIND_PROP(Item, sprite)
		PYBIND_PROP_SEPARATE(Item, stackSize, stack_size);

	PYBIND_DATA_CLASS(Tile, Prototype_base, tile)
		PYBIND_PROP_SEPARATE(Tile, isWater, is_water)
		PYBIND_PROP_SEPARATE(Tile, sprite, sprite_ptr);

	PYBIND_DATA_CLASS_SEPARATE(Noise_layer<Tile>, NoiseLayerTile, Prototype_base, noise_layer_tile)
		// Perlin noise properties
		PYBIND_PROP_SEPARATE(Noise_layer<Tile>, octaveCount, octave_count)
		PYBIND_PROP(Noise_layer<Tile>, frequency)
		PYBIND_PROP(Noise_layer<Tile>, persistence)

		PYBIND_PROP_SEPARATE(Noise_layer<Tile>, normalize, normalize_val)

		.def("getStartVal", &Noise_layer<Tile>::get_start_val)
		.def("startVal", &Noise_layer<Tile>::set_start_val)
		.def("add", &Noise_layer<Tile>::add)
		.def("get", &Noise_layer<Tile>::get);

	PYBIND_DATA_CLASS_SEPARATE(Noise_layer<Entity>, NoiseLayerEntity, Prototype_base, noise_layer_entity)
		// Perlin noise properties
		PYBIND_PROP_SEPARATE(Noise_layer<Entity>, octaveCount, octave_count)
		PYBIND_PROP(Noise_layer<Entity>, frequency)
		PYBIND_PROP(Noise_layer<Entity>, persistence)

		PYBIND_PROP_SEPARATE(Noise_layer<Entity>, normalize, normalize_val)

		.def("getStartVal", &Noise_layer<Entity>::get_start_val)
		.def("startVal", &Noise_layer<Entity>::set_start_val)
		.def("add", &Noise_layer<Entity>::add)
		.def("get", &Noise_layer<Entity>::get);
	

	// Entity
	py::class_<Entity, Prototype_base>(m, "Entity")
		PYBIND_PROP(Entity, sprite)
		PYBIND_PROP(Entity, rotatable)
		PYBIND_PROP_GET_SET(Entity, item, item)
		PYBIND_PROP_SEPARATE(Entity, tileWidth, tile_width)
		PYBIND_PROP_SEPARATE(Entity, tileHeight, tile_height)
		PYBIND_PROP_SEPARATE(Entity, pickupTime, pickup_time);

	PYBIND_DATA_CLASS_SEPARATE(Health_entity, HealthEntity, Entity, health_entity)
		PYBIND_PROP_SEPARATE(Health_entity, maxHealth, max_health);

	PYBIND_DATA_CLASS_SEPARATE(Container_entity, ContainerEntity, Health_entity, container_entity)
		PYBIND_PROP_SEPARATE(Container_entity, inventorySize, inventory_size);

	PYBIND_DATA_CLASS_SEPARATE(Resource_entity, ResourceEntity, Entity, resource_entity);
	
	// Recipes
	PYBIND_DATA_CLASS_SEPARATE(Recipe_group, RecipeGroup, Prototype_base, recipe_group)
		PYBIND_PROP(Recipe_group, sprite)
		PYBIND_PROP_SEPARATE(Recipe_group, recipeCategories, recipe_categories);

	PYBIND_DATA_CLASS_SEPARATE(Recipe_category, RecipeCategory, Prototype_base, recipe_category)
		PYBIND_PROP(Recipe_category, recipes);

	PYBIND_DATA_CLASS(Recipe, Prototype_base, recipe)
		PYBIND_PROP_SEPARATE(Recipe, craftingTime, crafting_time)
		PYBIND_PROP(Recipe, ingredients)
		PYBIND_PROP_GET_SET(Recipe, product, product);
	
	// ############################################################
	// Data_raw + get/set

	py::enum_<data_category>(m, "category")
		.value("Tile", data_category::tile)
		.value("Sprite", data_category::sprite)
		.value("NoiseLayerTile", data_category::noise_layer_tile)
		.value("NoiseLayerEntity", data_category::noise_layer_entity)
		.value("Sound", data_category::sound)
		.value("Item", data_category::item)

		.value("Recipe", data_category::recipe)
		.value("RecipeCategory", data_category::recipe_category)
		.value("RecipeGroup", data_category::recipe_group)

		.value("Entity", data_category::entity)
		.value("ResourceEntity", data_category::resource_entity)
		.value("EnemyEntity", data_category::enemy_entity)

		.value("HealthEntity", data_category::health_entity)
		.value("ContainerEntity", data_category::container_entity);

	m.def("get", [](const data_category category, const std::string& iname) {
		return data_manager::data_raw_get<Prototype_base>(category, iname);
	}, pybind11::return_value_policy::reference);

	// ############################################################
}

// Pybind binding macros should only be used in this file
#undef PYBIND_PROP
#undef PYBIND_PROP_SEPARATE
#undef PYBIND_PROP_GET_SET


#endif // DATA_PYBIND_PYBIND_BINDINGS_H
