// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_PROTO_RESOURCE_ENTITY_H
#define JACTORIO_INCLUDE_PROTO_RESOURCE_ENTITY_H
#pragma once

#include "core/data_type.h"
#include "proto/abstract/entity.h"

namespace jactorio::proto
{
    // Unique per resource entity placed
    struct ResourceEntityData final : EntityData
    {
        using ResourceCount = ResourceEntityResourceCount;

        /// Resource entity should never reach 0 resources, when it does it is treated as infinite
        static constexpr ResourceCount kInfiniteResource = 0;


        ResourceEntityData() = default;

        explicit ResourceEntityData(const ResourceCount resource_amount) : resourceAmount(resource_amount) {}


        /// Amount of product which can still be extracted from this tile
        ResourceCount resourceAmount = 1;


        CEREAL_SERIALIZE(archive) {
            archive(cereal::base_class<EntityData>(this), resourceAmount);
        }
    };


    class ResourceEntity final : public Entity
    {
    public:
        PROTOTYPE_CATEGORY(resource_entity);
        PROTOTYPE_DATA_TRIVIAL_COPY(ResourceEntityData);

        /// Seconds to pickup entity
        PYTHON_PROP_REF_I(float, pickupTime, 1);

        ResourceEntity() {
            // Resource entities can never be placed
            this->placeable = false;
        }


        void OnBuild(game::World& /*world*/,
                     game::Logic& /*logic*/,
                     const WorldCoord& /*coord*/,
                     game::TileLayer /*tlayer*/,
                     Orientation /*orientation*/) const override {
            assert(false); // Is not player placeable
        }

        void OnRemove(game::World& /*world*/,
                      game::Logic& /*logic*/,
                      const WorldCoord& /*coord*/,
                      game::TileLayer /*tlayer*/) const override {}


        void PostLoadValidate(const data::PrototypeManager& /*proto*/) const override {
            // Must convert to at least 1 game tick
            J_PROTO_ASSERT(pickupTime * kGameHertz >= 1, "Pickup time is too small");
        }
    };
} // namespace jactorio::proto

#endif // JACTORIO_INCLUDE_PROTO_RESOURCE_ENTITY_H
