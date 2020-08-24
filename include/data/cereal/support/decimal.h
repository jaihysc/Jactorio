// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_DATA_CEREAL_SUPPORT_DECIMAL_H
#define JACTORIO_DATA_CEREAL_SUPPORT_DECIMAL_H
#pragma once

#include "core/data_type.h"
#include "data/cereal/serialize.h"

namespace cereal
{
	CEREAL_LOAD_EXTERN(archive, jactorio::Decimal3T, m) {
		dec::int64 before_val;
		dec::int64 after_val;
		archive(before_val, after_val);

		m.pack(before_val, after_val);
	}

	CEREAL_SAVE_EXTERN(archive, jactorio::Decimal3T, m) {
		dec::int64 before_val;
		dec::int64 after_val;
		m.unpack(before_val, after_val);

		archive(before_val, after_val);
	}
}

#endif // JACTORIO_DATA_CEREAL_SUPPORT_DECIMAL_H
