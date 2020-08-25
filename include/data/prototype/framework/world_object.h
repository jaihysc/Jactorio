// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_DATA_PROTOTYPE_FRAMEWORK_WORLD_OBJECT_H
#define JACTORIO_DATA_PROTOTYPE_FRAMEWORK_WORLD_OBJECT_H
#pragma once

#include "data/prototype/framework/framework_base.h"
#include "data/prototype/interface/renderable.h"
#include "data/prototype/interface/serializable.h"

namespace jactorio::data
{
	struct FWorldObjectData : UniqueDataBase, IRenderableData
	{
		CEREAL_SERIALIZE(archive) {
			archive(cereal::base_class<UniqueDataBase>(this), cereal::base_class<IRenderableData>(this));
		}
	};

	class FWorldObject : public FrameworkBase, public IRenderable, public ISerializable
	{
	public:
		// Number of tiles this entity spans
		PYTHON_PROP_REF_I(uint8_t, tileWidth, 1);
		PYTHON_PROP_REF_I(uint8_t, tileHeight, 1);
	};
}

#endif // JACTORIO_DATA_PROTOTYPE_FRAMEWORK_WORLD_OBJECT_H