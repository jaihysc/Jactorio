// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "data/cereal/serialization_type.h"

#include "jactorioTests.h"
#include "data/prototype/sprite.h"
#include "data/prototype/item.h"

namespace jactorio::data
{
	TEST(CerealSerializePrototypePointer, ConstructPointer) {
		Sprite sprite;
		SerialProtoPtr<Sprite> serial_proto = &sprite;
		EXPECT_EQ(serial_proto.Get(), &sprite);
	}

	TEST(CerealSerializePrototypePointer, ConstructReference) {
		Sprite sprite;
		SerialProtoPtr<Sprite> serial_proto(sprite);
		EXPECT_EQ(serial_proto.Get(), &sprite);
	}

	TEST(CerealSerializePrototypePointer, GetSetPrototype) {
		SerialProtoPtr<Sprite> serial_proto;
		EXPECT_EQ(serial_proto.Get(), nullptr);

		Sprite sprite;
		serial_proto       = &sprite;
		serial_proto->sets = 3;
		EXPECT_EQ(sprite.sets, 3);

		(*serial_proto).sets = 4;
		EXPECT_EQ(sprite.sets, 4);
	}

	TEST(CerealSerializePrototypePointer, GetConstPrototype) {
		Sprite sprite;
		sprite.sets = 32;

		SerialProtoPtr<const Sprite> serial_proto(sprite);

		EXPECT_EQ(serial_proto->sets, 32);
		EXPECT_EQ((*serial_proto).sets, 32);
	}

	TEST(CerealSerializePrototypePointer, GetPrototypeConstWrapper) {
		Sprite sprite;
		sprite.sets = 32;

		const SerialProtoPtr<const Sprite> serial_proto(sprite);

		EXPECT_EQ(serial_proto->sets, 32);
		EXPECT_EQ((*serial_proto).sets, 32);
	}

	
	TEST(CerealSerializePrototypePointer, SerializeNull) {
		SerialProtoPtr<const Sprite> original(nullptr);

		auto result = TestSerializeDeserialize(original);
		EXPECT_EQ(result.Get(), nullptr);
	}
}
