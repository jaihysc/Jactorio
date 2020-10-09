// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_DATA_CEREAL_REGISTER_TYPE_H
#define JACTORIO_INCLUDE_DATA_CEREAL_REGISTER_TYPE_H
#pragma once

#include "proto/assembly_machine.h"
#include "proto/container_entity.h"
#include "proto/inserter.h"
#include "proto/mining_drill.h"
#include "proto/resource_entity.h"
#include "proto/transport_belt.h"

CEREAL_REGISTER_TYPE(jactorio::proto::ResourceEntityData);

CEREAL_REGISTER_TYPE(jactorio::proto::TransportLineData);

CEREAL_REGISTER_TYPE(jactorio::proto::ContainerEntityData);

CEREAL_REGISTER_TYPE(jactorio::proto::AssemblyMachineData);

CEREAL_REGISTER_TYPE(jactorio::proto::InserterData);

CEREAL_REGISTER_TYPE(jactorio::proto::MiningDrillData);

#endif // JACTORIO_INCLUDE_DATA_CEREAL_REGISTER_TYPE_H
