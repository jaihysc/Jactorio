// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_DATA_DATA_MANAGER_H
#define JACTORIO_INCLUDE_DATA_DATA_MANAGER_H
#pragma once

#include <algorithm>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "data/data_category.h"
#include "data/prototype/prototype_base.h"

namespace jactorio::data
{
	template <typename TProto>
	struct IsValidPrototype
	{
		static constexpr bool value =
			std::is_base_of_v<PrototypeBase, TProto> && !std::is_abstract_v<TProto>;
	};

	/// Pybind callbacks to append into the data manager at the pointer 
	/// SerialProtoPtr deserializes with this
	inline PrototypeManager* active_data_manager = nullptr;

	///
	/// \brief Manages prototype data
	class PrototypeManager
	{
		/// Position 0 reserved to indicate error
		static constexpr PrototypeIdT kInternalIdStart = 1;

		using RelocationTableContainerT = std::vector<const PrototypeBase*>;

		struct DebugInfo;

	public:
		PrototypeManager() = default;
		~PrototypeManager();

		PrototypeManager(const PrototypeManager& other)     = delete;
		PrototypeManager(PrototypeManager&& other) noexcept = delete;

		/// Path of the data folder from the executing directory
		static constexpr char kDataFolder[] = "data";

		// Get

		///
		/// \brief Gets prototype at specified name, cast to T
		/// \return nullptr if the specified prototype does not exist
		template <typename TProto,
		          std::enable_if_t<IsValidPrototype<TProto>::value, int>  = 0>
		TProto* DataRawGet(const std::string& iname) const noexcept;

		///
		/// \brief Gets prototype at specified category and name, cast to T
		/// \return nullptr if the specified prototype does not exist
		///
		/// Abstract types allowed for Python API
		template <typename TProto,
		          std::enable_if_t<std::is_base_of_v<PrototypeBase, TProto>, int>  = 0>
		TProto* DataRawGet(DataCategory data_category, const std::string& iname) const noexcept;


		///
		/// \brief Gets pointers to all data of specified data_type
		template <typename TProto,
		          std::enable_if_t<IsValidPrototype<TProto>::value, int>  = 0>
		std::vector<TProto*> DataRawGetAll(DataCategory type) const;

		///
		/// \brief Gets pointers to all data of specified data_type, sorted by Prototype_base.order
		template <typename TProto,
		          std::enable_if_t<IsValidPrototype<TProto>::value, int>  = 0>
		std::vector<TProto*> DataRawGetAllSorted(DataCategory type) const;


		// Add 

		///
		/// \brief Sets the prefix which will be added to all internal names
		/// Provide empty string to disable
		/// 
		/// Prefix of "base" : "electric-pole" becomes "__base__/electric-pole"
		void SetDirectoryPrefix(const std::string& name = "");

		///
		/// \brief Create anonymous prototype TProto
		/// \return Created prototype
		template <typename TProto,
		          std::enable_if_t<IsValidPrototype<TProto>::value, int>  = 0>
		TProto& AddProto() {
			return AddProto<TProto>("");
		}

		///
		/// \brief Forwards TArgs to create prototype TProto
		/// \return Created prototype
		template <typename TProto, typename ... TArgs,
		          std::enable_if_t<IsValidPrototype<TProto>::value, int>  = 0>
		TProto& AddProto(const std::string& iname, TArgs&& ... args);


		// Utility

		///
		/// \brief Searches through all categories for prototype
		/// \return pointer to prototype, nullptr if not found
		template <typename TProto = PrototypeBase,
		          std::enable_if_t<std::is_base_of_v<PrototypeBase, TProto>, int>  = 0>
		J_NODISCARD TProto* FindProto(const std::string& iname) const noexcept;


		// ======================================================================


		///
		/// \brief Loads data and their properties from data/ folder,
		/// \remark In normal usage, data access methods can be used only after calling this
		/// \param data_folder_path Do not include a / at the end (Valid usage: dc/xy/data)
		/// \exception DataException Prototype validation failed or Pybind error
		void LoadData(const std::string& data_folder_path);


		///
		/// \brief Frees all pointer data within data_raw, clears data_raw
		void ClearData();


		// ======================================================================


		// Deserialize

		///
		/// \brief RelocationTableGet can be used after this is called
		void GenerateRelocationTable();

		///
		/// \brief Fetches prototype at prototype id
		template <typename TProto = PrototypeBase,
		          std::enable_if_t<std::is_base_of_v<PrototypeBase, TProto>, int>  = 0>
		J_NODISCARD const TProto& RelocationTableGet(PrototypeIdT prototype_id) const noexcept;


		J_NODISCARD DebugInfo GetDebugInfo() const;

	private:
		///
		/// \brief Adds a prototype
		/// \param iname Internal name of prototype
		/// \param prototype Prototype pointer, takes ownership, must be unique for each added
		void DataRawAdd(const std::string& iname, PrototypeBase* prototype);


		struct DebugInfo
		{
			const RelocationTableContainerT& relocationTable;
		};


		/// Example: data_raw[static_cast<int>(image)]["grass-1"] -> Prototype_base
		std::unordered_map<std::string, PrototypeBase*> dataRaw_[static_cast<int>(DataCategory::count_)];

		RelocationTableContainerT relocationTable_;


		/// Internal id which will be assigned to the next prototype added
		PrototypeIdT internalIdNew_ = kInternalIdStart;

		/// Appended to the beginning of each new prototype
		std::string directoryPrefix_;
	};

	template <typename TProto,
	          std::enable_if_t<IsValidPrototype<TProto>::value, int>>
	TProto* PrototypeManager::DataRawGet(const std::string& iname) const noexcept {
		static_assert(TProto::category != DataCategory::none);

		return DataRawGet<TProto>(TProto::category, iname);
	}

	template <typename TProto,
	          std::enable_if_t<std::is_base_of_v<PrototypeBase, TProto>, int>>
	TProto* PrototypeManager::DataRawGet(const DataCategory data_category, const std::string& iname) const noexcept {

		auto* category = &dataRaw_[static_cast<uint16_t>(data_category)];
		if (category->find(iname) == category->end()) {
			LOG_MESSAGE_F(error, "Attempted to access non-existent prototype %s", iname.c_str());
			return nullptr;
		}

		PrototypeBase* base = category->at(iname);
		return static_cast<TProto*>(base);
	}

	template <typename TProto,
	          std::enable_if_t<IsValidPrototype<TProto>::value, int>>
	std::vector<TProto*> PrototypeManager::DataRawGetAll(const DataCategory type) const {
		auto category_items = dataRaw_[static_cast<uint16_t>(type)];

		std::vector<TProto*> items;
		items.reserve(category_items.size());

		for (auto& it : category_items) {
			PrototypeBase* base_ptr = it.second;
			items.push_back(static_cast<TProto*>(base_ptr));
		}

		return items;
	}

	template <typename TProto,
	          std::enable_if_t<IsValidPrototype<TProto>::value, int>>
	std::vector<TProto*> PrototypeManager::DataRawGetAllSorted(const DataCategory type) const {
		std::vector<TProto*> items = DataRawGetAll<TProto>(type);

		// Sort
		std::sort(items.begin(),
		          items.end(),
		          [](PrototypeBase* a, PrototypeBase* b) {
			          return a->order < b->order;
		          });
		return items;
	}

	template <typename TProto, typename ... TArgs,
	          std::enable_if_t<IsValidPrototype<TProto>::value, int>>
	TProto& PrototypeManager::AddProto(const std::string& iname, TArgs&&... args) {

		auto* proto = new TProto(std::forward<TArgs>(args)...);
		assert(proto != nullptr);

		DataRawAdd(iname, proto);
		return *proto;
	}

	template <typename TProto,
	          std::enable_if_t<std::is_base_of_v<PrototypeBase, TProto>, int>>
	TProto* PrototypeManager::FindProto(const std::string& iname) const noexcept {
		for (const auto& map : dataRaw_) {
			auto i = map.find(iname);
			if (i != map.end()) {
				return static_cast<TProto*>(i->second);
			}
		}
		return nullptr;
	}

	template <typename TProto, std::enable_if_t<std::is_base_of_v<PrototypeBase, TProto>, int>>
	const TProto& PrototypeManager::RelocationTableGet(const PrototypeIdT prototype_id) const noexcept {
		assert(relocationTable_.at(prototype_id));

		auto* proto = relocationTable_[prototype_id];
		return static_cast<const TProto&>(*proto);
	}
}

#endif //JACTORIO_INCLUDE_DATA_DATA_MANAGER_H
