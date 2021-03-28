// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_MANAGER_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_MANAGER_H
#pragma once

#include <algorithm>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "core/convert.h"
#include "proto/detail/category.h"
#include "proto/framework/framework_base.h"

namespace jactorio::data
{
    template <typename TProto>
    struct IsValidPrototype
    { static constexpr bool value = std::is_base_of_v<proto::FrameworkBase, TProto> && !std::is_abstract_v<TProto>; };

    /// Manages prototype data
    class PrototypeManager
    {
        /// Position 0 reserved to indicate error
        static constexpr PrototypeIdT kInternalIdStart_ = 1;

        using RelocationTableContainerT = std::vector<const proto::FrameworkBase*>;

        struct DebugInfo;

    public:
        PrototypeManager() = default;
        ~PrototypeManager();

        PrototypeManager(const PrototypeManager& other)     = delete;
        PrototypeManager(PrototypeManager&& other) noexcept = delete;

        /// Path of the data folder from the executing directory
        static constexpr char kDataFolder[] = "data";

        // Get

        /// Gets prototype at specified name, cast to T
        /// \return nullptr if the specified prototype does not exist
        template <typename TProto>
        TProto* Get(const std::string& iname) const noexcept;

        /// Gets prototype at specified category and name, cast to T
        /// \return nullptr if the specified prototype does not exist
        /// Abstract types allowed for Python API
        template <typename TProto>
        TProto* Get(proto::Category data_category, const std::string& iname) const noexcept;


        /// Gets pointers to all data of specified category
        template <typename TProto>
        std::vector<TProto*> GetAll(proto::Category category) const;
        /// Gets pointers to all data of category specified by TProto
        template <typename TProto>
        std::vector<TProto*> GetAll() const;

        /// Gets pointers to all data of specified category, sorted by FrameworkBase.order
        template <typename TProto>
        std::vector<TProto*> GetAllSorted(proto::Category category) const;
        /// Gets pointers to all data of category specified by TProto, sorted by FrameworkBase.order
        template <typename TProto>
        std::vector<TProto*> GetAllSorted() const;


        // Make

        /// Sets the prefix which will be added to all internal names
        /// Provide empty string to disable
        /// Prefix of "base" : "electric-pole" becomes "__base__/electric-pole"
        void SetDirectoryPrefix(const std::string& name = "");

        /// Create anonymous prototype TProto
        /// \return Created prototype
        template <typename TProto>
        TProto& Make() {
            return Make<TProto>("");
        }

        /// Forwards TArgs to create prototype TProto
        /// \return Created prototype
        template <typename TProto, typename... TArgs>
        TProto& Make(const std::string& iname, TArgs&&... args);


        // Utility

        /// Searches through all categories for prototype
        /// \return pointer to prototype, nullptr if not found
        template <typename TProto = proto::FrameworkBase>
        J_NODISCARD TProto* Find(const std::string& iname) const noexcept;


        // ======================================================================


        /// Loads prototypes and their properties from provided directory path
        /// \remark This is how the game loads prototypes normally
        /// \param folder_path Do not include a / at the end (Valid usage: dc/xy/data)
        /// \exception ProtoError Prototype validation failed or Pybind error
        void Load(const std::string& folder_path);


        /// Clears all prototype data
        void Clear();


        // ======================================================================


        // Deserialize

        /// RelocationTableGet can be used after this is called
        void GenerateRelocationTable();

        /// Fetches prototype at prototype id
        template <typename TProto = proto::FrameworkBase>
        J_NODISCARD const TProto& RelocationTableGet(PrototypeIdT prototype_id) const noexcept;


        J_NODISCARD DebugInfo GetDebugInfo() const;

    private:
        /// Adds a prototype
        /// \param iname Internal name of prototype
        /// \param prototype Prototype pointer, takes ownership, must be unique for each added
        void Add(const std::string& iname, proto::FrameworkBase* prototype);


        struct DebugInfo
        {
            const RelocationTableContainerT& relocationTable;
        };


        /// Example: data_raw[static_cast<int>(image)]["grass-1"] -> Prototype_base
        std::unordered_map<std::string, proto::FrameworkBase*> dataRaw_[static_cast<int>(proto::Category::count_)];

        RelocationTableContainerT relocationTable_;


        /// Internal id which will be assigned to the next prototype added
        PrototypeIdT internalIdNew_ = kInternalIdStart_;

        /// Appended to the beginning of each new prototype
        std::string directoryPrefix_;
    };

    template <typename TProto>
    TProto* PrototypeManager::Get(const std::string& iname) const noexcept {
        static_assert(IsValidPrototype<TProto>::value);
        static_assert(TProto::category != proto::Category::none);

        return Get<TProto>(TProto::category, iname);
    }

    template <typename TProto>
    TProto* PrototypeManager::Get(const proto::Category data_category, const std::string& iname) const noexcept {
        static_assert(std::is_base_of_v<proto::FrameworkBase, TProto>);

        const auto* category = &dataRaw_[static_cast<uint16_t>(data_category)];

        try {
            proto::FrameworkBase* base = category->at(iname);
            return SafeCast<TProto*>(base);
        }
        catch (std::out_of_range&) {
            LOG_MESSAGE_F(error, "Attempted to access non-existent prototype %s", iname.c_str());
            return nullptr;
        }
    }

    template <typename TProto>
    std::vector<TProto*> PrototypeManager::GetAll(const proto::Category category) const {
        static_assert(IsValidPrototype<TProto>::value);

        auto category_items = dataRaw_[static_cast<uint16_t>(category)];

        std::vector<TProto*> items;
        items.reserve(category_items.size());

        for (auto& it : category_items) {
            proto::FrameworkBase* base_ptr = it.second;
            items.push_back(SafeCast<TProto*>(base_ptr));
        }

        return items;
    }

    template <typename TProto>
    std::vector<TProto*> PrototypeManager::GetAll() const {
        return GetAll<TProto>(TProto::category);
    }

    template <typename TProto>
    std::vector<TProto*> PrototypeManager::GetAllSorted(const proto::Category category) const {
        static_assert(IsValidPrototype<TProto>::value);

        std::vector<TProto*> items = GetAll<TProto>(category);

        // Sort
        std::sort(items.begin(), items.end(), [](proto::FrameworkBase* a, proto::FrameworkBase* b) {
            return a->order < b->order;
        });
        return items;
    }

    template <typename TProto>
    std::vector<TProto*> PrototypeManager::GetAllSorted() const {
        return GetAllSorted<TProto>(TProto::category);
    }


    template <typename TProto, typename... TArgs>
    TProto& PrototypeManager::Make(const std::string& iname, TArgs&&... args) {
        static_assert(IsValidPrototype<TProto>::value);

        auto* proto = new TProto(std::forward<TArgs>(args)...);
        assert(proto != nullptr);

        Add(iname, proto);
        return *proto;
    }

    template <typename TProto>
    TProto* PrototypeManager::Find(const std::string& iname) const noexcept {
        static_assert(std::is_base_of_v<proto::FrameworkBase, TProto>);

        for (const auto& map : dataRaw_) {
            auto i = map.find(iname);
            if (i != map.end()) {
                return SafeCast<TProto*>(i->second);
            }
        }
        return nullptr;
    }

    template <typename TProto>
    const TProto& PrototypeManager::RelocationTableGet(PrototypeIdT prototype_id) const noexcept {
        static_assert(std::is_base_of_v<proto::FrameworkBase, TProto>);

        assert(relocationTable_.size() >= prototype_id);
        assert(prototype_id > 0);
        prototype_id -= 1; // Prototype ids start from 1

        assert(relocationTable_.at(prototype_id));

        const auto* proto = relocationTable_[prototype_id];
        return SafeCast<const TProto&>(*proto);
    }
} // namespace jactorio::data

#endif // JACTORIO_INCLUDE_DATA_PROTOTYPE_MANAGER_H
