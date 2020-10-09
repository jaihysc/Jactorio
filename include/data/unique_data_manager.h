// This file is subject to the terms and conditions defined in 'LICENSE' included in the source code package

#ifndef JACTORIO_INCLUDE_DATA_UNIQUE_DATA_MANAGER_H
#define JACTORIO_INCLUDE_DATA_UNIQUE_DATA_MANAGER_H
#pragma once

#include <vector>

#include "proto/framework/framework_base.h"

namespace jactorio::data
{
    class UniqueDataManager;

    inline UniqueDataManager* active_unique_data_manager = nullptr;

    class UniqueDataManager
    {
        static constexpr UniqueDataIdT kDefaultId = 1;

        using DataEntriesT = std::vector<proto::UniqueDataBase*>;

        struct DebugInfo;

    public:
        // For serializing
        void AssignId(proto::UniqueDataBase& framework_base) noexcept;


        // For deserializing
        void StoreRelocationEntry(proto::UniqueDataBase& unique_data);
        J_NODISCARD proto::UniqueDataBase& RelocationTableGet(UniqueDataIdT id) const noexcept;


        /// Resets internal data
        void RelocationClear() noexcept;


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
