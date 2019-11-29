#ifndef DATA_PROTOTYPE_PROTOTYPE_BASE_H
#define DATA_PROTOTYPE_PROTOTYPE_BASE_H

#include <string>

#include "data/data_category.h"

namespace jactorio::data
{
	class Prototype_base
	{
	public:
		/**
		 * Internal name <br>
		 * MUST BE unique per data_category
		 */
		std::string name;

		/**
		 * Category of this Prototype item
		 */
		data_category category;
	};
}

#endif // DATA_PROTOTYPE_PROTOTYPE_BASE_H
