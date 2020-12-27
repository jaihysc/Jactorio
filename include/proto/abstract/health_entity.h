// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_PROTO_ABSTRACT_HEALTH_ENTITY_H
#define JACTORIO_INCLUDE_PROTO_ABSTRACT_HEALTH_ENTITY_H
#pragma once

#include "proto/abstract/entity.h"

namespace jactorio::proto
{
    struct HealthEntityData : EntityData
    {
        uint16_t health = 0;


        CEREAL_SERIALIZE(archive) {
            archive(cereal::base_class<EntityData>(this), health);
        }
    };

    class HealthEntity : public Entity
    {
        /// Default health of all Health entities
        static constexpr uint16_t kDefaultHealth = 1;

    protected:
        HealthEntity() = default;

    public:
        ///
        /// How many hit points this entity can have before it dies
        /// \remark 0 max health is invalid
        PYTHON_PROP_REF_I(uint16_t, maxHealth, kDefaultHealth);

        void PostLoadValidate(const data::PrototypeManager& data_manager) const override {
            Entity::PostLoadValidate(data_manager);

            J_PROTO_ASSERT(maxHealth > 0, "Max health must be greater than 0");
        }
    };
} // namespace jactorio::proto

#endif // JACTORIO_INCLUDE_PROTO_ABSTRACT_HEALTH_ENTITY_H
