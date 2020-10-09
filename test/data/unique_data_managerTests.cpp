// This file is subject to the terms and conditions defined in 'LICENSE' included in the source code package

#include <gtest/gtest.h>

#include "proto/assembly_machine.h"

namespace jactorio::data
{
    TEST(UniqueDataManager, AssignId) {
        UniqueDataManager udm;

        AssemblyMachineData data_1;
        AssemblyMachineData data_2;

        udm.AssignId(data_1);
        udm.AssignId(data_2);

        EXPECT_EQ(data_1.internalId, 1);
        EXPECT_EQ(data_2.internalId, 2);
    }

    TEST(UniqueDataManager, ClearResetId) {
        UniqueDataManager udm;

        AssemblyMachineData data_1;
        udm.AssignId(data_1);

        udm.RelocationClear();
        AssemblyMachineData data_2;
        udm.AssignId(data_2);

        EXPECT_EQ(data_1.internalId, 1);
        EXPECT_EQ(data_2.internalId, 1);
    }

    TEST(UniqueDataManager, StoreGetRelocationEntry) {
        UniqueDataManager udm;

        AssemblyMachineData data_1;
        data_1.internalId = 1;
        udm.StoreRelocationEntry(data_1);


        AssemblyMachineData data_2;
        data_2.internalId = 10;
        udm.StoreRelocationEntry(data_2);

        EXPECT_EQ(&udm.RelocationTableGet(1), &data_1);
        EXPECT_EQ(&udm.RelocationTableGet(10), &data_2);
    }

    TEST(UniqueDataManager, StoreRelocationEntryOverwrite) {
        UniqueDataManager udm;

        AssemblyMachineData data_1;
        data_1.internalId = 1;
        udm.StoreRelocationEntry(data_1);


        AssemblyMachineData data_2;
        data_2.internalId = 1;
        udm.StoreRelocationEntry(data_2);

        EXPECT_EQ(&udm.RelocationTableGet(1), &data_2);
    }

    TEST(UniqueDataManager, ClearRelocationTable) {
        UniqueDataManager udm;

        AssemblyMachineData data_1;
        data_1.internalId = 1;
        udm.StoreRelocationEntry(data_1);

        udm.RelocationClear();

        EXPECT_TRUE(udm.GetDebugInfo().dataEntries.empty());
    }
} // namespace jactorio::data
