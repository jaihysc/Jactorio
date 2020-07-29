// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_DATA_PROTOTYPE_INTERFACE_PROTOTYPE_RENDERABLE_H
#define JACTORIO_DATA_PROTOTYPE_INTERFACE_PROTOTYPE_RENDERABLE_H
#pragma once

#include "data/prototype/prototype_base.h"
#include "data/prototype/interface/renderable.h"

namespace jactorio::data
{
	struct PrototypeRenderableData : UniqueDataBase, IRenderableData
	{
		PrototypeRenderableData() = default;

		explicit PrototypeRenderableData(const Sprite::SetT set)
			: IRenderableData(set) {
		}
	};

	///
	/// \brief Prototype which is renderable
	class IPrototypeRenderable : public PrototypeBase, public IRenderable
	{
	};
}

#endif // JACTORIO_DATA_PROTOTYPE_INTERFACE_PROTOTYPE_RENDERABLE_H
