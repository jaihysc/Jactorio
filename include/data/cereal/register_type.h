#ifndef JACTORIO_DATA_CEREAL_REGISTER_TYPE_H
#define JACTORIO_DATA_CEREAL_REGISTER_TYPE_H
#pragma once

#include "data/prototype/container_entity.h"
#include "data/prototype/resource_entity.h"
#include "data/prototype/transport_belt.h"

CEREAL_REGISTER_TYPE(jactorio::data::ResourceEntityData);

CEREAL_REGISTER_TYPE(jactorio::data::TransportLineData);

CEREAL_REGISTER_TYPE(jactorio::data::ContainerEntityData);

#endif // JACTORIO_DATA_CEREAL_REGISTER_TYPE_H
