// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 03/31/2020

#ifndef JACTORIO_INCLUDE_DATA_PROTOTYPE_ITEM_ITEM_BASE_H
#define JACTORIO_INCLUDE_DATA_PROTOTYPE_ITEM_ITEM_BASE_H
#pragma once

#include "data/prototype/prototype_base.h"
#include "data/prototype/sprite.h"

namespace jactorio::data
{
	///
	/// \brief Basic abstract class for items
	class ItemBase : public PrototypeBase
	{
	protected:
		ItemBase() = default;

		explicit ItemBase(Sprite* sprite)
			: sprite(sprite) {
		}

	public:
		PYTHON_PROP_I(ItemBase, Sprite*, sprite, nullptr);

		void PostLoadValidate(const PrototypeManager&) const override;
	};

	inline void ItemBase::PostLoadValidate(const PrototypeManager&) const {
		J_DATA_ASSERT(sprite != nullptr, "Sprite was not specified");
	}
}

#endif //JACTORIO_INCLUDE_DATA_PROTOTYPE_ITEM_ITEM_BASE_H
