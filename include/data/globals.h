// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_DATA_GLOBALS_H
#define JACTORIO_INCLUDE_DATA_GLOBALS_H
#pragma once

namespace jactorio::data
{
    class PrototypeManager;
    class UniqueDataManager;

    // Globals for jactorio::data namespace

    /// Pybind callbacks to append into the data manager at the pointer
    /// SerialProtoPtr deserializes with this
    inline PrototypeManager* active_prototype_manager    = nullptr;
    inline UniqueDataManager* active_unique_data_manager = nullptr;

} // namespace jactorio::data

#endif // JACTORIO_INCLUDE_DATA_GLOBALS_H
