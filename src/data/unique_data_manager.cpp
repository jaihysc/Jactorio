// This file is subject to the terms and conditions defined in 'LICENSE' included in the source code package

#include "data/unique_data_manager.h"

using namespace jactorio;

void data::UniqueDataManager::AssignId(UniqueDataBase& framework_base) noexcept {
    framework_base.internalId = nextId_;
    nextId_++;
}

// ======================================================================

void data::UniqueDataManager::StoreRelocationEntry(UniqueDataBase& unique_data) {
    assert(unique_data.internalId > 0);

    if (dataEntries_.size() < unique_data.internalId) {
        dataEntries_.resize(unique_data.internalId);
    }

    const auto index = unique_data.internalId - kDefaultId;
    assert(index < dataEntries_.size());
    dataEntries_[index] = &unique_data;
}

data::UniqueDataBase& data::UniqueDataManager::RelocationTableGet(const UniqueDataIdT id) const noexcept {
    assert(id > 0);
    return *dataEntries_[id - kDefaultId];
}

// ======================================================================

void data::UniqueDataManager::RelocationClear() noexcept {
    nextId_ = kDefaultId;
    dataEntries_.clear();
}

data::UniqueDataManager::DebugInfo data::UniqueDataManager::GetDebugInfo() {
    return {dataEntries_};
}
