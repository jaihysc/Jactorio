// 
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 11/09/2019

#ifndef JACTORIO_INCLUDE_DATA_PYBIND_PYBIND_BINDINGS_H
#define JACTORIO_INCLUDE_DATA_PYBIND_PYBIND_BINDINGS_H
#pragma once

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

#include "data/data_manager.h"
#include "data/prototype/entity/container_entity.h"
#include "data/prototype/entity/entity.h"
#include "data/prototype/entity/health_entity.h"
#include "data/prototype/entity/mining_drill.h"
#include "data/prototype/entity/resource_entity.h"
#include "data/prototype/entity/transport/transport_belt.h"
#include "data/prototype/entity/transport/transport_line.h"
#include "data/prototype/item/item.h"
#include "data/prototype/item/recipe_category.h"
#include "data/prototype/item/recipe_group.h"
#include "data/prototype/noise_layer.h"
#include "data/prototype/prototype_base.h"
#include "data/prototype/sprite.h"
#include "data/prototype/tile/tile.h"

// All the bindings in bindings/ defined for pybind
// This should only be included by pybind_manager.h

using data_raw = std::unordered_map<jactorio::data::dataCategory, std::unordered_map<
	                                    std::string, jactorio::data::Prototype_base*>>;

// Generates a function for constructing a data class
// Example: >Sprite<, inherits >Prototype_base< in category dataCategory::sprite
// 
// A python object with name _Sprite will be generated to avoid ambiguity
// To construct sprite, a constructor with the name Sprite(...) is available

#define PYBIND_DATA_CLASS(cpp_class_, py_name_, ...)\
	m.def(#py_name_, [](const std::string& iname = "") {\
		auto* prototype = new (cpp_class_);\
		data_manager::data_raw_add(iname, prototype, true);\
		return prototype;\
	}, py::arg("iname") = "", pybind11::return_value_policy::reference);\
	py::class_<cpp_class_, __VA_ARGS__>(m, "_" #py_name_)

// Does not define a function for creating the class
#define PYBIND_DATA_CLASS_ABSTRACT(cpp_class_, py_name_, inherits_)\
	py::class_<cpp_class_, inherits_>(m, "_" #py_name_)


#define PYBIND_DATA_INTERFACE(cpp_class_, py_name_)\
	py::class_<cpp_class_>(m, "_" #py_name_)

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
#define PYBIND_PROP_S(class_, py_name, cpp_name)\
	.def(#py_name, &class_::set_##cpp_name, pybind11::return_value_policy::reference)\
	.def_readwrite("_" #py_name, &class_::cpp_name)


// If the member utilizes a getter and settler
#define PYBIND_PROP_GET_SET(class_, py_name, cpp_name)\
	.def(#py_name, &class_::set_##cpp_name, pybind11::return_value_policy::reference)\
	.def("get_" #py_name, &class_::get_##cpp_name, pybind11::return_value_policy::reference)


PYBIND11_EMBEDDED_MODULE(jactorioData, m) {
	using namespace jactorio::data;

	// Interface classes
	// PYBIND_DATA_INTERFACE(Rotatable_entity, RotatableEntity)
	// 	PYBIND_PROP_S(Rotatable_entity, spriteE, sprite_e)
	// 	PYBIND_PROP_S(Rotatable_entity, spriteS, sprite_s)
	// 	PYBIND_PROP_S(Rotatable_entity, spriteW, sprite_w);


	// Prototype classes
	py::class_<Prototype_base>(m, "_PrototypeBase")
		.def("name", &Prototype_base::set_name)
		.def("category", &Prototype_base::category)
		PYBIND_PROP(Prototype_base, order);

	PYBIND_DATA_CLASS(Sprite, Sprite, Prototype_base)
		PYBIND_PROP(Sprite, group)
		PYBIND_PROP(Sprite, frames)
		PYBIND_PROP(Sprite, sets)
		PYBIND_PROP(Sprite, trim)
		.def("load", &Sprite::load_image);

	py::enum_<Sprite::spriteGroup>(m, "spriteGroup")
		.value("Terrain", Sprite::spriteGroup::terrain)
		.value("Gui", Sprite::spriteGroup::gui);

	PYBIND_DATA_CLASS(Item, Item, Prototype_base)
		PYBIND_PROP(Item, sprite)
		PYBIND_PROP_S(Item, stackSize, stack_size);

	PYBIND_DATA_CLASS(Tile, Tile, Prototype_base)
		PYBIND_PROP_S(Tile, isWater, is_water)
		PYBIND_PROP_S(Tile, sprite, sprite);

	PYBIND_DATA_CLASS(Noise_layer<Tile>, NoiseLayerTile, Prototype_base)
		// Perlin noise properties
		PYBIND_PROP_S(Noise_layer<Tile>, octaveCount, octave_count)
		PYBIND_PROP(Noise_layer<Tile>, frequency)
		PYBIND_PROP(Noise_layer<Tile>, persistence)

		PYBIND_PROP_S(Noise_layer<Tile>, normalize, normalize_val)

		.def("getStartVal", &Noise_layer<Tile>::get_start_val)
		.def("startVal", &Noise_layer<Tile>::set_start_val)
		.def("add", &Noise_layer<Tile>::add)
		.def("get", &Noise_layer<Tile>::get);

	PYBIND_DATA_CLASS(Noise_layer<Entity>, NoiseLayerEntity, Prototype_base)
		// Perlin noise properties
		PYBIND_PROP_S(Noise_layer<Entity>, octaveCount, octave_count)
		PYBIND_PROP(Noise_layer<Entity>, frequency)
		PYBIND_PROP(Noise_layer<Entity>, persistence)

		PYBIND_PROP_S(Noise_layer<Entity>, normalize, normalize_val)

		.def("getStartVal", &Noise_layer<Entity>::get_start_val)
		.def("startVal", &Noise_layer<Entity>::set_start_val)
		.def("add", &Noise_layer<Entity>::add)
		.def("get", &Noise_layer<Entity>::get);


	// Entity
	PYBIND_DATA_CLASS_ABSTRACT(Entity, Entity, Prototype_base)
		PYBIND_PROP(Entity, sprite)
		PYBIND_PROP_S(Rotatable, spriteE, sprite_e)
		PYBIND_PROP_S(Rotatable, spriteS, sprite_s)
		PYBIND_PROP_S(Rotatable, spriteW, sprite_w)
		PYBIND_PROP(Entity, rotatable)
		PYBIND_PROP(Entity, placeable)
		PYBIND_PROP_GET_SET(Entity, item, item)
		PYBIND_PROP_S(Entity, tileWidth, tile_width)
		PYBIND_PROP_S(Entity, tileHeight, tile_height)
		PYBIND_PROP_S(Entity, pickupTime, pickup_time);

	PYBIND_DATA_CLASS_ABSTRACT(Health_entity, HealthEntity, Entity)
		PYBIND_PROP_S(Health_entity, maxHealth, max_health);

	PYBIND_DATA_CLASS(Container_entity, ContainerEntity, Health_entity)
		PYBIND_PROP_S(Container_entity, inventorySize, inventory_size);

	PYBIND_DATA_CLASS(Resource_entity, ResourceEntity, Entity);


	// Belts
	PYBIND_DATA_CLASS_ABSTRACT(Transport_line, TransportLine, Health_entity)
		PYBIND_PROP_S(Transport_line, speed, speed_float);

	PYBIND_DATA_CLASS(Transport_belt, TransportBelt, Transport_line);

	// Mining drill
	PYBIND_DATA_CLASS(Mining_drill, MiningDrill, Health_entity)
		PYBIND_PROP_S(Mining_drill, miningSpeed, mining_speed);


	// Recipes
	PYBIND_DATA_CLASS(Recipe_group, RecipeGroup, Prototype_base)
		PYBIND_PROP(Recipe_group, sprite)
		PYBIND_PROP_S(Recipe_group, recipeCategories, recipe_categories);

	PYBIND_DATA_CLASS(Recipe_category, RecipeCategory, Prototype_base)
		PYBIND_PROP(Recipe_category, recipes);

	PYBIND_DATA_CLASS(Recipe, Recipe, Prototype_base)
		PYBIND_PROP_S(Recipe, craftingTime, crafting_time)
		PYBIND_PROP(Recipe, ingredients)
		PYBIND_PROP_GET_SET(Recipe, product, product);

	// ############################################################
	// Data_raw + get/set

	py::enum_<dataCategory>(m, "category")
		.value("Tile", dataCategory::tile)
		.value("Sprite", dataCategory::sprite)
		.value("NoiseLayerTile", dataCategory::noise_layer_tile)
		.value("NoiseLayerEntity", dataCategory::noise_layer_entity)
		.value("Sound", dataCategory::sound)
		.value("Item", dataCategory::item)

		.value("Recipe", dataCategory::recipe)
		.value("RecipeCategory", dataCategory::recipe_category)
		.value("RecipeGroup", dataCategory::recipe_group)

		// .value("Entity", dataCategory::entity)
		.value("ResourceEntity", dataCategory::resource_entity)
		.value("EnemyEntity", dataCategory::enemy_entity)

		// .value("HealthEntity", dataCategory::health_entity)
		.value("ContainerEntity", dataCategory::container_entity)

		.value("TransportBelt", dataCategory::transport_belt)
		.value("MiningDrill", dataCategory::transport_belt);

	m.def("get", [](const dataCategory category, const std::string& iname) {
		return data_manager::data_raw_get<Prototype_base>(category, iname);
	}, pybind11::return_value_policy::reference);

	// ############################################################
}

// Pybind binding macros should only be used in this file
#undef PYBIND_PROP
#undef PYBIND_PROP_S
#undef PYBIND_PROP_GET_SET


#endif //JACTORIO_INCLUDE_DATA_PYBIND_PYBIND_BINDINGS_H
