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
#include "data/prototype/tile/enemy_tile.h"
#include "data/prototype/tile/resource_tile.h"
#include "data/prototype/tile/tile.h"
#include "data/prototype/item/recipe_group.h"
#include "data/prototype/item/recipe_category.h"

// All the bindings in bindings/ defined for pybind
// This should only be included by pybind_manager.h

using data_raw = std::unordered_map<jactorio::data::data_category, std::unordered_map<
	                                    std::string, jactorio::data::Prototype_base>>;

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

	py::class_<Sprite, Prototype_base>(m, "Sprite")
		PYBIND_PROP(Sprite, group)
		.def("load", &Sprite::load_image);
	py::enum_<Sprite::sprite_group>(m, "spriteGroup")
		.value("Terrain", Sprite::sprite_group::terrain)
		.value("Gui", Sprite::sprite_group::gui);

	py::class_<Item, Prototype_base>(m, "Item")
		PYBIND_PROP(Item, sprite)
		PYBIND_PROP_SEPARATE(Item, stackSize, stack_size);

	py::class_<Tile, Prototype_base>(m, "Tile")
		PYBIND_PROP_SEPARATE(Tile, isWater, is_water)
		PYBIND_PROP_SEPARATE(Tile, sprite, sprite_ptr);

	py::class_<Resource_tile, Tile>(m, "ResourceTile")
		PYBIND_PROP(Resource_tile, product);

	py::class_<Noise_layer, Prototype_base>(m, "NoiseLayer")
		// Perlin noise properties
		PYBIND_PROP_SEPARATE(Noise_layer, octaveCount, octave_count)
		PYBIND_PROP(Noise_layer, frequency)
		PYBIND_PROP(Noise_layer, persistence)

		PYBIND_PROP_SEPARATE(Noise_layer, tileDataCategory, tile_data_category)
		PYBIND_PROP_SEPARATE(Noise_layer, normalize, normalize_val)

		.def("getStartVal", &Noise_layer::get_start_val)
		.def("startVal", &Noise_layer::set_start_val)
		.def("addTile", &Noise_layer::add_tile)
		.def("getTile", &Noise_layer::get_tile);


	// Entity
	py::class_<Entity, Prototype_base>(m, "Entity")
		PYBIND_PROP(Entity, sprite)
		PYBIND_PROP(Entity, rotatable)
		PYBIND_PROP_GET_SET(Entity, item, item)
		PYBIND_PROP_SEPARATE(Entity, tileWidth, tile_width)
		PYBIND_PROP_SEPARATE(Entity, tileHeight, tile_height);

	py::class_<Health_entity, Entity>(m, "HealthEntity")
		PYBIND_PROP_SEPARATE(Health_entity, maxHealth, max_health);

	py::class_<Container_entity, Health_entity>(m, "ContainerEntity")
		PYBIND_PROP_SEPARATE(Container_entity, inventorySize, inventory_size);

	// Recipes
	py::class_<Recipe_group, Prototype_base>(m, "RecipeGroup")
		PYBIND_PROP(Recipe_group, sprite)
		PYBIND_PROP_SEPARATE(Recipe_group, recipeCategories, recipe_categories);

	py::class_<Recipe_category, Prototype_base>(m, "RecipeCategory")
		PYBIND_PROP(Recipe_category, recipes);

	py::class_<Recipe, Prototype_base>(m, "Recipe")
		PYBIND_PROP(Recipe, ingredients)
		PYBIND_PROP_GET_SET(Recipe, product, product);
	
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

		.value("Recipe", data_category::recipe)
		.value("RecipeCategory", data_category::recipe_category)
		.value("RecipeGroup", data_category::recipe_group)

		.value("Entity", data_category::entity)
		.value("HealthEntity", data_category::health_entity)
		.value("ContainerEntity", data_category::container_entity);

	py::class_<data_raw>(m, "Prototype_data")
		.def(py::init());

	m.def("get", [](const data_category category, const std::string& iname) {
		return data_manager::data_raw_get<Prototype_base>(category, iname);
	}, pybind11::return_value_policy::reference);


#define PROTOTYPE_CATEGORY(category, class_) case data_category::category: prototype = new (class_); break;
	/**
	 * Call add from python to retrieve a new pointer to the prototype <br>
	 * Prototypes cannot be made in Python as they need to be copied back to c++
	 */
	m.def("add", [](const data_category category, const std::string& iname = "") -> Prototype_base* {
		assert(category != data_category::none);
		
		Prototype_base* prototype = nullptr;
		
		switch (category) {
			// Maps enum to a class which is constructed and returned
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

			PROTOTYPE_CATEGORY(recipe, Recipe);
			PROTOTYPE_CATEGORY(recipe_category, Recipe_category);
			PROTOTYPE_CATEGORY(recipe_group, Recipe_group);
		
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

// Pybind binding macros should only be used in this file
#undef PYBIND_PROP
#undef PYBIND_PROP_SEPARATE
#undef PYBIND_PROP_GET_SET


#endif // DATA_PYBIND_PYBIND_BINDINGS_H
