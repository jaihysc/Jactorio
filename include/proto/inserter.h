// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_PROTO_INSERTER_H
#define JACTORIO_INCLUDE_PROTO_INSERTER_H
#pragma once

#include <decimal.h>

#include "game/logic/inserter_controller.h"
#include "game/logic/item_logistics.h"
#include "game/logistic/inventory.h"
#include "proto/abstract/health_entity.h"

namespace jactorio::proto
{
    /// Holds the internal structure for inserters
    struct InserterData final : HealthEntityData
    {
        explicit InserterData(const Orientation orientation)
            : orientation(orientation), dropoff(orientation), pickup(orientation) {}

        enum class Status
        {
            dropoff,
            pickup
        };

        // Orientation points towards dropoff
        Orientation orientation;

        /// Rotation degree of current inserter, 0 is dropoff, 180 is pickup
        RotationDegreeT rotationDegree = RotationDegreeT(game::kMaxInserterDegree);

        /// Current inserter status
        Status status = Status::pickup;

        /// Current item held by inserter
        game::ItemStack heldItem;

        game::ItemDropOff dropoff;
        game::InserterPickup pickup;


        CEREAL_SERIALIZE(archive) {
            archive(orientation, rotationDegree, status, heldItem, cereal::base_class<HealthEntityData>(this));
        }

        CEREAL_LOAD_CONSTRUCT(archive, construct, InserterData) {
            Orientation orientation;
            archive(orientation);
            construct(orientation);

            archive(construct->rotationDegree,
                    construct->status,
                    construct->heldItem,
                    cereal::base_class<HealthEntityData>(construct.ptr()));
        }
    };


    class Inserter final : public HealthEntity
    {
    public:
        PROTOTYPE_CATEGORY(inserter);
        PROTOTYPE_DATA_TRIVIAL_COPY(InserterData);

        /// Part closer to the base
        PYTHON_PROP_I(Sprite*, armSprite, nullptr);
        /// The hand holding the item
        PYTHON_PROP_I(Sprite*, handSprite, nullptr);

        /// Degrees to rotate per tick
        /// \remark For Python API use only
        PYTHON_PROP_I(ProtoFloatT, rotationSpeedFloat, 0.1);

        /// Tile distance which the inserter can reach
        PYTHON_PROP_I(ProtoUintT, tileReach, 1);

        /// Degrees to rotate per tick
        RotationDegreeT rotationSpeed;


        void PostLoad() override {
            rotationSpeed = RotationDegreeT(rotationSpeedFloat);
        }


        // ======================================================================

        void OnRDrawUniqueData(render::RendererLayer& layer,
                               const SpriteTexCoords& uv_coords,
                               const Position2<float>& pixel_offset,
                               const UniqueDataBase* unique_data) const override;

        J_NODISCARD SpriteSetT OnRGetSpriteSet(Orientation orientation,
                                               game::World& world,
                                               const WorldCoord& coord) const override;

        /// \param orientation Points towards dropoff
        void OnBuild(game::World& world,
                     game::Logic& logic,
                     const WorldCoord& coord,
                     game::TileLayer tlayer,
                     Orientation orientation) const override;


        void OnTileUpdate(game::World& world,
                          const WorldCoord& emit_coord,
                          const WorldCoord& receive_coord,
                          UpdateType type) const override;

        void OnRemove(game::World& world,
                      game::Logic& logic,
                      const WorldCoord& coord,
                      game::TileLayer tlayer) const override;


        void OnDeserialize(game::World& world, const WorldCoord& coord, game::ChunkTile& tile) const override;


        void PostLoadValidate(const data::PrototypeManager& proto) const override;
        void ValidatedPostLoad() override;

    private:
        J_NODISCARD WorldCoord GetDropoffCoord(const WorldCoord& coord, Orientation orientation) const;
        J_NODISCARD WorldCoord GetPickupCoord(const WorldCoord& coord, Orientation orientation) const;

        void InitPickupDropoff(game::World& world, const WorldCoord& coord, Orientation orientation) const;
    };
} // namespace jactorio::proto

#endif // JACTORIO_INCLUDE_PROTO_INSERTER_H
