#ifndef DATA_PROTOTYPE_DATA_CATEGORY_H
#define DATA_PROTOTYPE_DATA_CATEGORY_H

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

#endif // DATA_PROTOTYPE_DATA_CATEGORY_H
