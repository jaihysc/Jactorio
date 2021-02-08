// This file is subject to the terms and conditions defined in 'LICENSE' included in the source code package

#include <gtest/gtest.h>

#include "proto/assembly_machine.h"

namespace jactorio::data
{
    TEST(UniqueDataManager, AssignId) {
        UniqueDataManager unique;

        proto::AssemblyMachineData data_1;
        proto::AssemblyMachineData data_2;

        unique.AssignId(data_1);
        unique.AssignId(data_2);

        EXPECT_EQ(data_1.internalId, 1);
        EXPECT_EQ(data_2.internalId, 2);
    }

    TEST(UniqueDataManager, ClearResetId) {
        UniqueDataManager unique;

        proto::AssemblyMachineData data_1;
        unique.AssignId(data_1);

        unique.Clear();
        proto::AssemblyMachineData data_2;
        unique.AssignId(data_2);

        EXPECT_EQ(data_1.internalId, 1);
        EXPECT_EQ(data_2.internalId, 1);
    }

    TEST(UniqueDataManager, StoreGetRelocationEntry) {
        UniqueDataManager unique;

        proto::AssemblyMachineData data_1;
        data_1.internalId = 1;
        unique.StoreRelocationEntry(data_1);


        proto::AssemblyMachineData data_2;
        data_2.internalId = 10;
        unique.StoreRelocationEntry(data_2);

        EXPECT_EQ(&unique.RelocationTableGet(1), &data_1);
        EXPECT_EQ(&unique.RelocationTableGet(10), &data_2);
    }

    TEST(UniqueDataManager, StoreRelocationEntryOverwrite) {
        UniqueDataManager unique;

        proto::AssemblyMachineData data_1;
        data_1.internalId = 1;
        unique.StoreRelocationEntry(data_1);


        proto::AssemblyMachineData data_2;
        data_2.internalId = 1;
        unique.StoreRelocationEntry(data_2);

        EXPECT_EQ(&unique.RelocationTableGet(1), &data_2);
    }

    TEST(UniqueDataManager, ClearRelocationTable) {
        UniqueDataManager unique;

        proto::AssemblyMachineData data_1;
        data_1.internalId = 1;
        unique.StoreRelocationEntry(data_1);

        unique.Clear();

        EXPECT_TRUE(unique.GetDebugInfo().dataEntries.empty());
    }
} // namespace jactorio::data
