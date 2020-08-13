// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include <gtest/gtest.h>

#include "data/cereal/serialization_type.h"
#include "data/prototype/sprite.h"

namespace jactorio::data
{
	TEST(CerealSerializePrototypePointer, GetSetPrototype) {
		SerialProtoPtr<Sprite> serial_proto;

		// .Get
		Sprite* sprite_ptr = serial_proto.Get();
		EXPECT_EQ(sprite_ptr, nullptr);

		// operator =
		Sprite sprite;
		serial_proto = &sprite;
		EXPECT_EQ(serial_proto.Get(), &sprite);

		// operator ->
		serial_proto->sets = 3;
		EXPECT_EQ(sprite.sets, 3);

		// operator *
		(*serial_proto).sets;
		EXPECT_EQ(sprite.sets, 3);
	}
	
}
