#include <gtest/gtest.h>

#include "data/prototype/container_entity.h"

#include "jactorioTests.h"

namespace jactorio::data
{
    TEST(ContainerEntity, Serialize) {
        ContainerEntityData container(5);
        container.health = 43;
        container.set    = 16;

        container.inventory[2].count = 89;


        PrototypeManager proto_manager;
        auto& item_1 = proto_manager.AddProto<Item>();
        auto& item_2 = proto_manager.AddProto<Item>();

        container.inventory[4].item   = &item_1;
        container.inventory[4].filter = &item_2;

        proto_manager.GenerateRelocationTable();
        active_prototype_manager = &proto_manager;

        // ======================================================================
        const auto result = TestSerializeDeserialize(container);

        EXPECT_EQ(result.health, 43);
        EXPECT_EQ(result.set, 16);
        EXPECT_EQ(result.set, 16);

        EXPECT_EQ(result.inventory[2].count, 89);

        EXPECT_EQ(result.inventory[4].item, &item_1);
        EXPECT_EQ(result.inventory[4].filter, &item_2);
    }
} // namespace jactorio::data
