// 
// data_category.h
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 03/14/2020
// Last modified: 03/14/2020
// 

#ifndef JACTORIO_INCLUDE_DATA_DATA_CATEGORY_H
#define JACTORIO_INCLUDE_DATA_DATA_CATEGORY_H
#pragma once

namespace jactorio::data
{
	enum class data_category
	{
		none = 0,
		tile,
		sprite,
		noise_layer_tile,
		noise_layer_entity,
		sound,
		item,

		entity,
		resource_entity,
		enemy_entity,

		health_entity,
		container_entity,

		transport_belt,

		recipe,
		recipe_category,
		recipe_group,

		count_
	};
}

#endif //JACTORIO_INCLUDE_DATA_DATA_CATEGORY_H
