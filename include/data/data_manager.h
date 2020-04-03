// 
// data_manager.h
// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// 
// Created on: 10/22/2019
// Last modified: 04/03/2020
// 

#ifndef JACTORIO_INCLUDE_DATA_DATA_MANAGER_H
#define JACTORIO_INCLUDE_DATA_DATA_MANAGER_H
#pragma once

#include <unordered_map>
#include <vector>
#include <algorithm>

#include "data/data_category.h"
#include "data/prototype/prototype_base.h"
#include "core/logger.h"

namespace jactorio::data
{
	/**
	* Restricted symbols in internal name; for internal use only: #
	*/
	namespace data_manager
	{
		// Path of the data folder from the executing directory
		constexpr char data_folder[] = "data";

		// Example: data_raw[static_cast<int>(image)]["grass-1"] -> Prototype_base

		inline std::unordered_map<std::string, Prototype_base*> data_raw[static_cast<int>(dataCategory::count_)];

		// Data_raw functions

		/**
		 * Gets prototype at specified category and name, is casted to T for convenience <br>
		 * Ensure that the casted type is or a parent of the specified category
		 * @return nullptr if the specified prototype does not exist
		 */
		template <typename T>
		T* data_raw_get(const dataCategory data_category, const std::string& iname) {
			auto category = &data_raw[static_cast<uint16_t>(data_category)];
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
		std::vector<T*> data_raw_get_all(const dataCategory type) {
			auto category_items = data_raw[static_cast<uint16_t>(type)];

			std::vector<T*> items;
			items.reserve(category_items.size());

			for (auto& it : category_items) {
				Prototype_base* base_ptr = it.second;
				items.push_back(static_cast<T*>(base_ptr));
			}

			return items;
		}

		/**
		 * Gets pointers to all data of specified data_type, sorted by Prototype_base.order
		 */
		template <typename T>
		std::vector<T*> data_raw_get_all_sorted(const dataCategory type) {
			std::vector<T*> items = data_raw_get_all<T>(type);

			// Sort
			std::sort(items.begin(), items.end(), [](Prototype_base* a, Prototype_base* b) {
				return a->order < b->order;
			});
			return items;
		}

		/**
		 * Sets the prefix which will be added to all internal names <br>
		 * Prefix of "base"
		 * "electric-pole" becomes "__base__/electric-pole"
		 */
		void set_directory_prefix(const std::string& name);

		///
		/// \brief Adds a prototype
		/// \param data_category
		/// \param iname Internal name of prototype
		/// \param prototype Prototype pointer, do not delete, must be unique for each added
		/// \param add_directory_prefix Should the directory prefix be appended to the provided iname
		void data_raw_add(dataCategory data_category, const std::string& iname, Prototype_base* prototype,
		                  bool add_directory_prefix = false);


		/**
		 * Loads data and their properties from data/ folder,
		 * data access methods can be used only after calling this
		 * @param data_folder_path Do not include a / at the end (Valid usage: dc/xy/data)
		 * @exception Data_exception Prototype validation failed or Pybind error
		 */
		void load_data(const std::string& data_folder_path);

		/**
		 * Frees all pointer data within data_raw, clears data_raw
		 */
		void clear_data();
	};
}

#endif //JACTORIO_INCLUDE_DATA_DATA_MANAGER_H
