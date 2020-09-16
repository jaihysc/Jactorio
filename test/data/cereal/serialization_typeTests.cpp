// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "data/cereal/serialization_type.h"

#include "jactorioTests.h"

namespace jactorio::data
{
    TEST(SerialPrototypePointer, Serialize) {
        PrototypeManager proto_manager;

        auto& container = proto_manager.AddProto<ContainerEntity>();

        SerialProtoPtr<const ContainerEntity> original(&container);


        proto_manager.GenerateRelocationTable();
        active_prototype_manager = &proto_manager;

        auto result = TestSerializeDeserialize(original);


        EXPECT_EQ(result.Get()->internalId, 1);
        EXPECT_TRUE(original == result);
        EXPECT_EQ(original.Get(), result.Get());
    }

    TEST(SerialPrototypePointer, SerializeNull) {
        const SerialProtoPtr<const Sprite> original(nullptr);

        EXPECT_TRUE(original == SerialProtoPtr<const Sprite>(nullptr));
        EXPECT_FALSE(original != SerialProtoPtr<const Sprite>(nullptr));

        auto result = TestSerializeDeserialize(original);
        EXPECT_EQ(result.Get(), nullptr);
    }

    TEST(SerialUniqueDataPointer, Serialize) {
        UniqueDataManager unique_manager;

        ContainerEntityData container;
        container.health = 42;
        unique_manager.AssignId(container);
        unique_manager.StoreRelocationEntry(container);

        const SerialUniqueDataPtr serial_ptr(&container);


        active_unique_data_manager = &unique_manager;
        auto result                = TestSerializeDeserialize(serial_ptr);

        EXPECT_EQ(result->internalId, 1);
        EXPECT_EQ(result->health, 42);
    }

    TEST(SerialUniqueDataPointer, SerializeNull) {
        const SerialUniqueDataPtr<ContainerEntityData> serial_ptr(nullptr);

        auto result = TestSerializeDeserialize(serial_ptr);

        EXPECT_EQ(result.Get(), nullptr);
    }
} // namespace jactorio::data
