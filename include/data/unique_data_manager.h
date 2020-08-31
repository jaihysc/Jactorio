// This file is subject to the terms and conditions defined in 'LICENSE' included in the source code package

#ifndef JACTORIO_INCLUDE_DATA_UNIQUE_DATA_MANAGER_H
#define JACTORIO_INCLUDE_DATA_UNIQUE_DATA_MANAGER_H
#pragma once

#include <vector>

#include "data/prototype/framework/framework_base.h"

namespace jactorio::data
{
    class UniqueDataManager
    {
        static constexpr UniqueDataIdT kDefaultId_ = 1;

        using DataEntriesT = std::vector<UniqueDataBase*>;

        struct DebugInfo;

    public:
        // For serializing
        void AssignId(UniqueDataBase& framework_base) noexcept;


        // For deserializing
        void StoreRelocationEntry(UniqueDataBase& unique_data);
        J_NODISCARD UniqueDataBase& RelocationTableGet(UniqueDataIdT id) const noexcept;


        /// Resets internal data
        void RelocationClear() noexcept;


        DebugInfo GetDebugInfo();

    private:
        struct DebugInfo
        {
            DataEntriesT& dataEntries;
        };

        UniqueDataIdT nextId_ = kDefaultId_;
        DataEntriesT dataEntries_;
    };
} // namespace jactorio::data

#endif // JACTORIO_INCLUDE_DATA_UNIQUE_DATA_MANAGER_H
