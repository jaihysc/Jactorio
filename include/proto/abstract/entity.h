// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_PROTO_ABSTRACT_ENTITY_H
#define JACTORIO_INCLUDE_PROTO_ABSTRACT_ENTITY_H
#pragma once

#include "jactorio.h"

#include "game/world/tile_layer.h"
#include "proto/detail/type.h"
#include "proto/framework/entity.h"

namespace jactorio::proto
{
    class Item;

    /// Unique per entity placed in the world
    struct EntityData : FEntityData
    {
        CEREAL_SERIALIZE(archive) {
            archive(cereal::base_class<FEntityData>(this));
        }
    };

    /// Placeable items in the world
    class Entity : public FEntity
    {
    public:
        Entity() = default;

        ~Entity() override = default;

        Entity(const Entity& other)     = default;
        Entity(Entity&& other) noexcept = default;

        Entity& operator=(const Entity& other) = default;
        Entity& operator=(Entity&& other) noexcept = default;


        /// Can be rotated by player?
        /// If false, also sets rotateDimensions to false
        PYTHON_PROP_REF_I(bool, rotatable, false);
        /// Can be placed by player?
        PYTHON_PROP_REF_I(bool, placeable, true);

        /// Item for this entity
        J_NODISCARD Item* GetItem() const {
            return item_;
        }

        Entity* SetItem(Item* item);


        // ======================================================================
        // Localized names

        // Overrides the default setter to also set for the item associated with this
        void SetLocalizedName(const std::string& localized_name) override;
        void SetLocalizedDescription(const std::string& localized_description) override;


        // Renderer events

        bool OnRShowGui(const gui::Context& /*context*/, game::ChunkTile* /*tile*/) const override {
            return false;
        }

        // Game events

        /// Entity was build in the world
        virtual void OnBuild(game::World& world,
                             game::Logic& logic,
                             const WorldCoord& coord,
                             Orientation orientation) const = 0;

        /// Determines if prototype can be built at coord
        /// \param coord Top left of prototype
        /// \param orien Orientation of prototype
        /// \return true if can be built
        J_NODISCARD virtual bool OnCanBuild(const game::World& world,
                                            const WorldCoord& coord,
                                            const Orientation orien) const {
            return true;
        }


        /// Entity was picked up from a built state, called BEFORE the entity has been removed
        virtual void OnRemove(game::World& world, game::Logic& logic, const WorldCoord& coord) const = 0;

        /// A neighbor of this prototype in the world was updated
        /// \param emit_coords Coordinates of the prototype which is EMITTING the update
        /// \param receive_coords Coordinates of the prototype RECEIVING the update
        /// \param emit_orientation Orientation to the prototype EMITTING the update
        virtual void OnNeighborUpdate(game::World& /*world*/,
                                      game::Logic& /*logic*/,
                                      const WorldCoord& emit_coords,
                                      const WorldCoord& receive_coords,
                                      Orientation emit_orientation) const {}


        void OnDeferTimeElapsed(game::World& /*world*/,
                                game::Logic& /*logic*/,
                                UniqueDataBase* /*unique_data*/) const override {
            assert(false); // Unimplemented
        }

        void OnTileUpdate(game::World& /*world*/,
                          const WorldCoord& /*emit_coords*/,
                          const WorldCoord& /*receive_coords*/,
                          UpdateType /*type*/) const override {
            assert(false); // Unimplemented
        }

        /// \param coord Top left coordinate
        /// \param tile Tile in world at coord
        void OnDeserialize(game::World& world, const WorldCoord& coord, game::ChunkTile& tile) const override {}


        void PostLoad() override;

        void PostLoadValidate(const data::PrototypeManager& proto) const override;

        void SetupSprite() override;

    private:
        /// Item when entity is picked up
        Item* item_ = nullptr;
    };
} // namespace jactorio::proto

#endif // JACTORIO_INCLUDE_PROTO_ABSTRACT_ENTITY_H
