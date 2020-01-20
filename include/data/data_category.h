#ifndef DATA_PROTOTYPE_DATA_CATEGORY_H
#define DATA_PROTOTYPE_DATA_CATEGORY_H

namespace jactorio::data
{
	enum class data_category
	{
		none = 0,
		tile,
		resource_tile,
		enemy_tile,
		sprite,
		noise_layer,
		sound,
		item,
		
		entity,
		health_entity,
		container_entity,

		recipe,
		recipe_category,
		recipe_group,
	};
}

#endif // DATA_PROTOTYPE_DATA_CATEGORY_H
