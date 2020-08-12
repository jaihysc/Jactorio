// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_DATA_PYBIND_PYBIND_BINDINGS_H
#define JACTORIO_INCLUDE_DATA_PYBIND_PYBIND_BINDINGS_H
#pragma once

#include <algorithm>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

#include "data/prototype_manager.h"
#include "data/prototype/prototype_base.h"
#include "data/prototype/prototype_type.h"
#include "data/prototype/sprite.h"
#include "data/prototype/entity/assembly_machine.h"
#include "data/prototype/entity/container_entity.h"
#include "data/prototype/entity/entity.h"
#include "data/prototype/entity/health_entity.h"
#include "data/prototype/entity/inserter.h"
#include "data/prototype/entity/mining_drill.h"
#include "data/prototype/entity/resource_entity.h"
#include "data/prototype/entity/transport_belt.h"
#include "data/prototype/entity/transport_line.h"
#include "data/prototype/item/item.h"
#include "data/prototype/item/recipe_category.h"
#include "data/prototype/item/recipe_group.h"
#include "data/prototype/tile/noise_layer.h"
#include "data/prototype/tile/tile.h"

// All the bindings in bindings/ defined for pybind
// This should only be included by pybind_manager.h

// Generates a function for constructing a data class
// Example: >Sprite<, inherits >Prototype_base< in category dataCategory::sprite
// 
// A python object with name _Sprite will be generated to avoid ambiguity
// To construct sprite, a constructor with the name Sprite(...) is available


///
/// \brief Call from python context, stores traceback in prototype
inline void ExtractPythonTraceback(jactorio::data::PrototypeBase& prototype) {
	py::exec(
		"import sys as _sys \n"
		"_stack_frame = _sys._getframe() \n"
	);

	std::vector<std::string> stack_frames{};
	
	while (py::eval("_stack_frame != None").cast<bool>()) {
		stack_frames.push_back(static_cast<py::str>(py::eval("_stack_frame")));

		py::exec("_stack_frame = _stack_frame.f_back");
	}

	// Reverse as most recent call is last
	std::reverse(stack_frames.begin(), stack_frames.end());

	for (auto& stack_frame : stack_frames) {
		prototype.pythonTraceback.append(stack_frame);
		prototype.pythonTraceback.append("\n");
	}
	prototype.pythonTraceback.pop_back();  // Remove final newline
}

#define PYBIND_DATA_CLASS(cpp_class_, py_name_, ...)\
	m.def(#py_name_, [](const std::string& iname = "") {\
		auto* prototype = new (cpp_class_);\
		\
		ExtractPythonTraceback(*prototype);\
		\
		assert(active_data_manager);\
		active_data_manager->DataRawAdd(iname, prototype, true);\
		\
		return prototype;\
	}, py::arg("iname") = "", pybind11::return_value_policy::reference);\
	py::class_<cpp_class_, __VA_ARGS__>(m, "_" #py_name_)

// Does not define a function for creating the class
#define PYBIND_DATA_CLASS_ABSTRACT(cpp_class_, py_name_, inherits_)\
	py::class_<cpp_class_, inherits_>(m, "_" #py_name_)


#define PYBIND_TYPE_CLASS(cpp_class_, py_name_)\
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
#define PYBIND_PROP(class_, name_)\
	.def          (#name_    , &class_::Set_##name_, pybind11::return_value_policy::reference)\
	.def_readwrite("_" #name_, &class_::name_)

// If the python name is different from the c++ name
#define PYBIND_PROP_S(class_, py_name_, cpp_name_, cpp_name_set_)\
	.def          (#py_name_    , &class_::cpp_name_set_, pybind11::return_value_policy::reference)\
	.def_readwrite("_" #py_name_, &class_::cpp_name_)


// If the member utilizes a getter and settler
#define PYBIND_PROP_GET_SET(class_, py_name_, cpp_name_set_, cpp_name_get_)\
	.def(#py_name_       , &class_::cpp_name_set_, pybind11::return_value_policy::reference)\
	.def("get_" #py_name_, &class_::cpp_name_get_, pybind11::return_value_policy::reference)

// ======================================================================

PYBIND11_EMBEDDED_MODULE(jactorioData, m) {
	using namespace jactorio::data;

	// Type 
	PYBIND_TYPE_CLASS(Tile4Way, OutputTile4Way)
		.def(py::init<
			std::tuple<
				std::pair<jactorio::WorldCoordAxis, jactorio::WorldCoordAxis>,
				std::pair<jactorio::WorldCoordAxis, jactorio::WorldCoordAxis>,
				std::pair<jactorio::WorldCoordAxis, jactorio::WorldCoordAxis>,
				std::pair<jactorio::WorldCoordAxis, jactorio::WorldCoordAxis>
			>>())
		.def_readwrite("up", &Tile4Way::up)
		.def_readwrite("right", &Tile4Way::right)
		.def_readwrite("down", &Tile4Way::down)
		.def_readwrite("left", &Tile4Way::left);


	// Prototype classes
	py::class_<PrototypeBase>(m, "_PrototypeBase")
		.def("name", &PrototypeBase::Set_name)
		.def("category", &PrototypeBase::Category)
		PYBIND_PROP(PrototypeBase, order);

	PYBIND_DATA_CLASS(Sprite, Sprite, PrototypeBase)
		PYBIND_PROP(Sprite, group)
		PYBIND_PROP(Sprite, frames)
		PYBIND_PROP(Sprite, sets)
		PYBIND_PROP(Sprite, trim)
		PYBIND_PROP(Sprite, invertSetFrame)
		.def("load", &Sprite::LoadImage);

	py::enum_<Sprite::SpriteGroup>(m, "spriteGroup")
		.value("Terrain", Sprite::SpriteGroup::terrain)
		.value("Gui", Sprite::SpriteGroup::gui);

	PYBIND_DATA_CLASS(Item, Item, PrototypeBase)
		PYBIND_PROP(Item, sprite)
		PYBIND_PROP(Item, stackSize);

	PYBIND_DATA_CLASS(Tile, Tile, PrototypeBase)
		PYBIND_PROP(Tile, isWater)
		PYBIND_PROP(Tile, sprite);

	PYBIND_DATA_CLASS(NoiseLayer<Tile>, NoiseLayerTile, PrototypeBase)
		// Perlin noise properties
		PYBIND_PROP(NoiseLayer<Tile>, octaveCount)
		PYBIND_PROP(NoiseLayer<Tile>, frequency)
		PYBIND_PROP(NoiseLayer<Tile>, persistence)

		PYBIND_PROP(NoiseLayer<Tile>, normalize)
		PYBIND_PROP(NoiseLayer<Tile>, richness)

		.def("getStartNoise", &NoiseLayer<Tile>::GetStartNoise)
		.def("startNoise", &NoiseLayer<Tile>::SetStartNoise)
		.def("add", &NoiseLayer<Tile>::Add)
		.def("get", &NoiseLayer<Tile>::Get);

	PYBIND_DATA_CLASS(NoiseLayer<Entity>, NoiseLayerEntity, PrototypeBase)
		// Perlin noise properties
		PYBIND_PROP(NoiseLayer<Entity>, octaveCount)
		PYBIND_PROP(NoiseLayer<Entity>, frequency)
		PYBIND_PROP(NoiseLayer<Entity>, persistence)

		PYBIND_PROP(NoiseLayer<Entity>, normalize)
		PYBIND_PROP(NoiseLayer<Entity>, richness)

		.def("getStartNoise", &NoiseLayer<Entity>::GetStartNoise)
		.def("startNoise", &NoiseLayer<Entity>::SetStartNoise)
		.def("add", &NoiseLayer<Entity>::Add)
		.def("get", &NoiseLayer<Entity>::Get);


	// Entity
	PYBIND_DATA_CLASS_ABSTRACT(Entity, Entity, PrototypeBase)
		PYBIND_PROP(Entity, sprite)
		PYBIND_PROP(IRotatable, spriteE)
		PYBIND_PROP(IRotatable, spriteS)
		PYBIND_PROP(IRotatable, spriteW)
		PYBIND_PROP(Entity, rotatable)
		PYBIND_PROP(Entity, placeable)
		PYBIND_PROP_GET_SET(Entity, item, SetItem, GetItem)
		PYBIND_PROP(Entity, tileWidth)
		PYBIND_PROP(Entity, tileHeight)
		PYBIND_PROP(Entity, pickupTime);

	PYBIND_DATA_CLASS_ABSTRACT(HealthEntity, HealthEntity, Entity)
		PYBIND_PROP(HealthEntity, maxHealth);

	PYBIND_DATA_CLASS(ContainerEntity, ContainerEntity, HealthEntity)
		PYBIND_PROP(ContainerEntity, inventorySize);

	PYBIND_DATA_CLASS(ResourceEntity, ResourceEntity, Entity);


	// Belts
	PYBIND_DATA_CLASS_ABSTRACT(TransportLine, TransportLine, HealthEntity)
		PYBIND_PROP_S(TransportLine, speed, speedFloat, Set_speedFloat);

	//
	PYBIND_DATA_CLASS(TransportBelt, TransportBelt, TransportLine);

	// Assembly machine
	PYBIND_DATA_CLASS(AssemblyMachine, AssemblyMachine, HealthEntity)
		PYBIND_PROP(AssemblyMachine, assemblySpeed);

	// Mining drill
	PYBIND_DATA_CLASS(MiningDrill, MiningDrill, HealthEntity)
		PYBIND_PROP(MiningDrill, miningSpeed)
		PYBIND_PROP(MiningDrill, resourceOutput);

	// Inserter
	PYBIND_DATA_CLASS(Inserter, Inserter, HealthEntity)
		PYBIND_PROP(Inserter, handSprite)
		PYBIND_PROP(Inserter, armSprite)
		PYBIND_PROP_S(Inserter, rotationSpeed, rotationSpeedFloat, Set_rotationSpeedFloat)
		PYBIND_PROP(Inserter, tileReach);


	// Recipes
	PYBIND_DATA_CLASS(RecipeGroup, RecipeGroup, PrototypeBase)
		PYBIND_PROP_S(RecipeGroup, sprite, sprite, SetSprite)
		PYBIND_PROP(RecipeGroup, recipeCategories);

	PYBIND_DATA_CLASS(RecipeCategory, RecipeCategory, PrototypeBase)
		PYBIND_PROP(RecipeCategory, recipes);

	PYBIND_DATA_CLASS(Recipe, Recipe, PrototypeBase)
		PYBIND_PROP(Recipe, craftingTime)
		PYBIND_PROP(Recipe, ingredients)
		PYBIND_PROP(Recipe, product);

	// ############################################################
	// Data_raw + get/set

	py::enum_<DataCategory>(m, "category")
		.value("Tile", DataCategory::tile)
		.value("Sprite", DataCategory::sprite)
		.value("NoiseLayerTile", DataCategory::noise_layer_tile)
		.value("NoiseLayerEntity", DataCategory::noise_layer_entity)
		.value("Sound", DataCategory::sound)
		.value("Item", DataCategory::item)

		.value("Recipe", DataCategory::recipe)
		.value("RecipeCategory", DataCategory::recipe_category)
		.value("RecipeGroup", DataCategory::recipe_group)

		.value("ResourceEntity", DataCategory::resource_entity)
		.value("EnemyEntity", DataCategory::enemy_entity)

		.value("ContainerEntity", DataCategory::container_entity)
		.value("AssemblyMachine", DataCategory::assembly_machine)
		.value("TransportBelt", DataCategory::transport_belt)
		.value("MiningDrill", DataCategory::mining_drill)
		.value("Inserter", DataCategory::inserter);

	m.def("get", [](const DataCategory category, const std::string& iname) {
		assert(active_data_manager);

		return active_data_manager->DataRawGet<PrototypeBase>(category, iname);
	}, pybind11::return_value_policy::reference);

	// ############################################################
}

// Pybind binding macros should only be used in this file
#undef PYBIND_PROP
#undef PYBIND_PROP_S
#undef PYBIND_PROP_GET_SET


#endif //JACTORIO_INCLUDE_DATA_PYBIND_PYBIND_BINDINGS_H
