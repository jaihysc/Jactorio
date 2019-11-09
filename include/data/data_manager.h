#ifndef DATA_DATA_MANAGER_H
#define DATA_DATA_MANAGER_H

#include <unordered_map>
#include <vector>
#include "data/prototype/prototype_base.h"

namespace jactorio::data
{
	enum class data_category
	{
		tile,
		audio
	};

	/** Accessed via Hdata access methods
	* std::string type -> Everything of specified type (E.g image, audio)
	* >> std::string id - > std::string path to item
	*/
	namespace data_manager
	{
		// Data_raw functions
		Prototype_base* data_raw_get(data_category data_category, const std::string& iname);
		void data_raw_add(data_category data_type, const std::string& iname, const Prototype_base& prototype);
		
		/**
		 * Loads data and their properties from data/ folder,
		 * data access methods can be used only after calling this
		 * @param data_folder_path Do not include a / at the end (Valid usage: dc/xy/data)
		 */
		void load_data(const std::string& data_folder_path);

		// Data access methods
		/**
		 * Retrieves non resolved path to file based on type and id <br>
		 * @return string "!" if specified value does not exist
		 */
		std::string get_path(data_category type, const std::string& iname);

		/**
		 * Gets all data of specified data_type
		 */
		std::vector<Prototype_base> get_all_data(data_category type);
	};
}

#endif // DATA_DATA_MANAGER_H
