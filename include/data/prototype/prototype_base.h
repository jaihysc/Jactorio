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
		 * Unique per prototype, auto assigned per new prototype added, faster than
		 * std::string name <br>J
		 * 0 indicates invalid id
		 */
		unsigned int internal_id = 0;
		
		/**
		 * Internal name <br>
		 * MUST BE unique per data_category
		 */
		std::string name;

		/**
		 * Category of this Prototype item
		 */
		data_category category;

		/**
		 * Determines the priority of this prototype, used in certain situations, see documentation
		 * within inheritors <br<
		 * Automatically assigned incrementally alongside internal_id if not defined <br>
		 * 0 indicates invalid id
		 */
		unsigned int order = 0;
	};
}

#endif // DATA_PROTOTYPE_PROTOTYPE_BASE_H
