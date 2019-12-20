#ifndef DATA_DATA_MANAGER_H
#define DATA_DATA_MANAGER_H

#include <unordered_map>
#include <vector>

#include "data/data_category.h"
#include "data/prototype/prototype_base.h"
#include "core/logger.h"

namespace jactorio::data
{
	// Internally used internal names
	// game-renderer-spritemap	Used by spritemap_generator.cpp to store the game's spritemap
	
	/**
	* std::string type -> Everything of specified type (E.g image, audio)
	* >> std::string id - > std::string path to item
	* See header for a list of already used internal names
	*/
	namespace data_manager
	{
		// Example: data_raw[image]["grass-1"] -> Prototype_base

		inline std::unordered_map<data_category, std::unordered_map<
			                          std::string, Prototype_base*>> data_raw;
		
		// Data_raw functions
		
		/**
		 * Gets prototype at specified category and name, is casted to T for convenience <br>
		 * Ensure that the casted type is or a parent of the specified category
		 * @return nullptr if the specified prototype does not exist
		 */
		template <typename T>
		T* data_raw_get(const data_category data_category, const std::string& iname) {
			auto category = &data_raw[data_category];
			if (category->find(iname) == category->end()) {
				LOG_MESSAGE_f(error, "Attempted to access non-existent prototype %s", iname.c_str());
				return nullptr;
			}

			// Address of prototype item downcasted to T
			Prototype_base* base = category->at(iname);
			return static_cast<T*>(base);
		}
		
		/**
		 * Gets pointers to all data of specified data_type
		 */
		template <typename T>
		std::vector<T*> data_raw_get_all(const data_category type) {
			auto category_items = data_raw[type];

			std::vector<T*> items;
			items.reserve(category_items.size());

			for (auto& it : category_items) {
				Prototype_base* base_ptr = it.second;
				items.push_back(static_cast<T*>(base_ptr));
			}

			return items;
		}
		

		/**
		 * Sets the prefix which will be added to all internal names <br>
		 * Prefix of "base"
		 * "electric-pole" becomes "__base__/electric-pole"
		 */
		void set_directory_prefix(const std::string& name);
		
		void data_raw_add(data_category data_category, const std::string& iname, Prototype_base* prototype);

		
		/**
		 * Loads data and their properties from data/ folder,
		 * data access methods can be used only after calling this
		 * @param data_folder_path Do not include a / at the end (Valid usage: dc/xy/data)
		 */
		void load_data(const std::string& data_folder_path);

		/**
		 * Frees all pointer data within data_raw, clears data_raw
		 */
		void clear_data();
	};
}

#endif // DATA_DATA_MANAGER_H
