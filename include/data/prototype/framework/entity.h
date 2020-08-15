// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_DATA_PROTOTYPE_FRAMEWORK_ENTITY_H
#define JACTORIO_DATA_PROTOTYPE_FRAMEWORK_ENTITY_H
#pragma once

#include "data/prototype/framework/renderable.h"
#include "data/prototype/interface/deferred.h"
#include "data/prototype/interface/rotatable.h"

namespace jactorio::data
{
	struct FEntityData : FRenderableData
	{
	};

	class FEntity : public FRenderable, public IRotatable, public IDeferred, public IUpdateListener
	{
	};
}

#endif // JACTORIO_DATA_PROTOTYPE_FRAMEWORK_ENTITY_H
