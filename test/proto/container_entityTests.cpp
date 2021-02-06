#include <gtest/gtest.h>

#include "proto/container_entity.h"

#include "jactorioTests.h"

namespace jactorio::proto
{
    TEST(ContainerEntity, Serialize) {
        ContainerEntityData container(5);
        container.health = 43;
        container.set    = 16;

        container.inventory[2].count = 89;


        data::PrototypeManager proto;
        auto& item_1 = proto.Make<Item>();
        auto& item_2 = proto.Make<Item>();

        container.inventory[4].item   = &item_1;
        container.inventory[4].filter = &item_2;

        proto.GenerateRelocationTable();
        data::active_prototype_manager = &proto;

        // ======================================================================
        const auto result = TestSerializeDeserialize(container);

        EXPECT_EQ(result.health, 43);
        EXPECT_EQ(result.set, 16);
        EXPECT_EQ(result.set, 16);

        EXPECT_EQ(result.inventory[2].count, 89);

        EXPECT_EQ(result.inventory[4].item, &item_1);
        EXPECT_EQ(result.inventory[4].filter, &item_2);
    }
} // namespace jactorio::proto
