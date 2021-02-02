// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_PROTO_CONTAINER_ENTITY_H
#define JACTORIO_INCLUDE_PROTO_CONTAINER_ENTITY_H
#pragma once

#include "proto/abstract/health_entity.h"
#include "proto/item.h"

namespace jactorio::proto
{
    struct ContainerEntityData final : HealthEntityData
    {
        ContainerEntityData() = default;

        explicit ContainerEntityData(const uint16_t inventory_size) {
            inventory.resize(inventory_size);
        }

        explicit ContainerEntityData(Item::Inventory inv) : inventory(std::move(inv)) {}

        Item::Inventory inventory;

        CEREAL_SERIALIZE(archive) {
            archive(cereal::base_class<HealthEntityData>(this), inventory);
        }
    };

    ///
    /// An entity with an inventory, such as a chest
    class ContainerEntity final : public HealthEntity
    {
    public:
        PROTOTYPE_CATEGORY(container_entity);
        PROTOTYPE_DATA_TRIVIAL_COPY(ContainerEntityData);

        ContainerEntity() : inventorySize(0) {}

        PYTHON_PROP_REF(uint16_t, inventorySize);


        // Events

        void OnBuild(game::World& world,
                     game::LogicData& logic_data,
                     const WorldCoord& world_coords,
                     game::ChunkTileLayer& tile_layer,
                     Orientation orientation) const override;

        void OnRemove(game::World& /*world*/,
                      game::LogicData& /*logic_data*/,
                      const WorldCoord& /*world_coords*/,
                      game::ChunkTileLayer& /*tile_layer*/) const override {}

        bool OnRShowGui(const render::GuiRenderer& g_rendr, game::ChunkTileLayer* tile_layer) const override;

        void ValidatedPostLoad() override;
    };
} // namespace jactorio::proto

#endif // JACTORIO_INCLUDE_PROTO_CONTAINER_ENTITY_H
