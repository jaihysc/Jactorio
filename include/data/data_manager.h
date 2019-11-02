#ifndef DATA_DATA_MANAGER_H
#define DATA_DATA_MANAGER_H

#include <unordered_map>
#include <vector>

namespace jactorio::data
{
	enum class data_type
	{
		// Used to indicate failure in data resolution
		none,
		// PNG
		graphics,
		// OGG
		audio
	};

	namespace data_manager
	{
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
		std::string get_path(data_type type, const std::string& iname);
		/**
		 * Retrieves internal name based on non resolved path <br>
		 * The opposite of get_path <br>
		 * @return string "!" if specified value does not exist
		 */
		std::string get_iname(data_type type, const std::string& path);

		/**
		 * Gets all data of specified data_type
		 */
		std::vector<std::string> get_all_data(data_type type);
	};
}

#endif // DATA_DATA_MANAGER_H
