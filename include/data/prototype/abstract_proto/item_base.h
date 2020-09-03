// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_ITEM_ITEM_BASE_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_ITEM_ITEM_BASE_H
#pragma once

#include "data/prototype/sprite.h"
#include "data/prototype/framework/framework_base.h"

namespace jactorio::data
{
	///
	/// \brief Basic abstract class for items
	class ItemBase : public FrameworkBase
	{
	protected:
		ItemBase() = default;

		explicit ItemBase(Sprite* sprite)
			: sprite(sprite) {
		}

	public:
		PYTHON_PROP_I(Sprite*, sprite, nullptr);

		void PostLoadValidate(const PrototypeManager&) const override;
	};

	inline void ItemBase::PostLoadValidate(const PrototypeManager&) const {
		J_DATA_ASSERT(sprite != nullptr, "Sprite was not specified");
	}
}

#endif //JACTORIO_INCLUDE_DATA_PROTOTYPE_ITEM_ITEM_BASE_H
