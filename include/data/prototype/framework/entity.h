// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_DATA_PROTOTYPE_FRAMEWORK_ENTITY_H
#define JACTORIO_DATA_PROTOTYPE_FRAMEWORK_ENTITY_H
#pragma once

#include "data/prototype/framework/renderable.h"
#include "data/prototype/interface/deferred.h"
#include "data/prototype/interface/rotatable.h"
#include "data/prototype/interface/update_listener.h"

namespace jactorio::data
{
	struct FEntityData : FRenderableData
	{
		CEREAL_SERIALIZE(archive) {
			archive(cereal::base_class<FRenderableData>(this));
		}
	};

	class FEntity : public FRenderable, public IRotatable, public IDeferred, public IUpdateListener
	{
	};
}

#endif // JACTORIO_DATA_PROTOTYPE_FRAMEWORK_ENTITY_H
