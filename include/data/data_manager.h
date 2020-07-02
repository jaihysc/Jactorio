// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 10/22/2019

#ifndef JACTORIO_INCLUDE_DATA_DATA_MANAGER_H
#define JACTORIO_INCLUDE_DATA_DATA_MANAGER_H
#pragma once

#include <algorithm>
#include <unordered_map>
#include <vector>

#include "data/data_category.h"
#include "data/prototype/prototype_base.h"

namespace jactorio::data
{
	///
	/// \brief Manages prototype data
	class DataManager
	{
	public:
		DataManager() = default;
		~DataManager();

		DataManager(const DataManager& other)     = delete;
		DataManager(DataManager&& other) noexcept = delete;

	private:
		/// Position 0 reserved to indicate error
		static constexpr auto internal_id_start = 1;

		/// Internal id which will be assigned to the next prototype added
		unsigned int internalIdNew_ = internal_id_start;

		/// Appended to the beginning of each new prototype
		std::string directoryPrefix_;

	public:
		/// Path of the data folder from the executing directory
		static constexpr char kDataFolder[] = "data";

		/// Example: data_raw[static_cast<int>(image)]["grass-1"] -> Prototype_base
		std::unordered_map<std::string, PrototypeBase*> dataRaw[static_cast<int>(DataCategory::count_)];


		///
		/// \brief Gets prototype at specified category and name, is casted to T for convenience <br>
		/// \remark Ensure that the casted type is or a parent of the specified category
		/// \return nullptr if the specified prototype does not exist
		template <typename T>
		T* DataRawGet(const DataCategory data_category, const std::string& iname) const {
			auto* category = &dataRaw[static_cast<uint16_t>(data_category)];
			if (category->find(iname) == category->end()) {
				LOG_MESSAGE_F(error, "Attempted to access non-existent prototype %s", iname.c_str());
				return nullptr;
			}

			// Address of prototype item downcasted to T
			PrototypeBase* base = category->at(iname);
			return static_cast<T*>(base);
		}

		///
		/// \brief Gets pointers to all data of specified data_type
		template <typename T>
		std::vector<T*> DataRawGetAll(const DataCategory type) const {
			auto category_items = dataRaw[static_cast<uint16_t>(type)];

			std::vector<T*> items;
			items.reserve(category_items.size());

			for (auto& it : category_items) {
				PrototypeBase* base_ptr = it.second;
				items.push_back(static_cast<T*>(base_ptr));
			}

			return items;
		}

		///
		/// \brief Gets pointers to all data of specified data_type, sorted by Prototype_base.order
		template <typename T>
		std::vector<T*> DataRawGetAllSorted(const DataCategory type) const {
			std::vector<T*> items = DataRawGetAll<T>(type);

			// Sort
			std::sort(items.begin(),
			          items.end(),
			          [](PrototypeBase* a, PrototypeBase* b) {
				          return a->order < b->order;
			          });
			return items;
		}

		// ======================================================================

		///
		/// \brief Sets the prefix which will be added to all internal names <br>
		/// Prefix of "base" : "electric-pole" becomes "__base__/electric-pole"
		void SetDirectoryPrefix(const std::string& name);

		///
		/// \brief Adds a prototype
		/// \param iname Internal name of prototype
		/// \param prototype Prototype pointer, do not delete, must be unique for each added
		/// \param add_directory_prefix Should the directory prefix be appended to the provided iname
		void DataRawAdd(const std::string& iname,
		                PrototypeBase* prototype,
		                bool add_directory_prefix = false);


		///
		/// \brief Loads data and their properties from data/ folder,
		/// \remark In normal usage, data access methods can be used only after calling this
		/// \param data_folder_path Do not include a / at the end (Valid usage: dc/xy/data)
		/// \exception DataException Prototype validation failed or Pybind error
		void LoadData(const std::string& data_folder_path);


		///
		/// \brief Searches through all categories to check if prototype exists, perfer DataRawGet() == nullptr if category is known
		J_NODISCARD bool PrototypeExists(const std::string& iname) const;

		///
		/// \brief Frees all pointer data within data_raw, clears data_raw
		void ClearData();
	};


	/// For pybind callbacks to append into the data manager at the pointer 
	inline DataManager* active_data_manager = nullptr;
}

#endif //JACTORIO_INCLUDE_DATA_DATA_MANAGER_H
