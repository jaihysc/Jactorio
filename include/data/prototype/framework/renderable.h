// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_DATA_PROTOTYPE_FRAMEWORK_RENDERABLE_H
#define JACTORIO_DATA_PROTOTYPE_FRAMEWORK_RENDERABLE_H
#pragma once

#include "data/prototype/framework/framework_base.h"
#include "data/prototype/interface/renderable.h"

namespace jactorio::data
{
	struct FRenderableData : UniqueDataBase, IRenderableData
	{
	};

	class FRenderable : public FrameworkBase, public IRenderable
	{
	};
}

#endif // JACTORIO_DATA_PROTOTYPE_FRAMEWORK_RENDERABLE_H
