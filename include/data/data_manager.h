#ifndef DATA_DATA_MANAGER_H
#define DATA_DATA_MANAGER_H

#include <unordered_map>

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
		 * Returns string "!" if specified value does not exist
		 */
		std::string get_data(data_type type, const std::string& id);

		/**
		 * Same as get_data
		 * Does not do bounds checking, prone to errors if not careful!
		 */
		//            static std::string get_data_f(data_type type, std::string id);
	};
}

#endif // DATA_DATA_MANAGER_H
