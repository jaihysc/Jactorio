// This file is subject to the terms and conditions defined in 'LICENSE' included in the source code package

#ifndef JACTORIO_INCLUDE_DATA_UNIQUE_DATA_MANAGER_H
#define JACTORIO_INCLUDE_DATA_UNIQUE_DATA_MANAGER_H
#pragma once

#include <vector>

#include "proto/framework/framework_base.h"

namespace jactorio::data
{
    class UniqueDataManager;

    class UniqueDataManager
    {
        static constexpr UniqueDataIdT kDefaultId = 1;

        using DataEntriesT = std::vector<proto::UniqueDataBase*>;

        struct DebugInfo;

    public:
        // To serialize:
        // 1. Call AssignId() with the unique data
        // 2. Serialize the assigned id to unique_data

        // To deserialize:
        // 1. Deserialize all unique data first, call StoreRelocationEntry() with each
        // 2. Call RelocationTableGet with serialized id to retrieve unique data

        void AssignId(proto::UniqueDataBase& unique_data) noexcept;

        void StoreRelocationEntry(proto::UniqueDataBase& unique_data);
        J_NODISCARD proto::UniqueDataBase& RelocationTableGet(UniqueDataIdT id) const noexcept;


        /// Clears the relocation table
        void Clear() noexcept;


        DebugInfo GetDebugInfo();

    private:
        struct DebugInfo
        {
            DataEntriesT& dataEntries;
        };

        UniqueDataIdT nextId_ = kDefaultId;
        DataEntriesT dataEntries_;
    };
} // namespace jactorio::data

#endif // JACTORIO_INCLUDE_DATA_UNIQUE_DATA_MANAGER_H
