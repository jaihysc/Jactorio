// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_PROTO_ABSTRACT_ENTITY_H
#define JACTORIO_INCLUDE_PROTO_ABSTRACT_ENTITY_H
#pragma once

#include "jactorio.h"

#include "proto/detail/type.h"
#include "proto/framework/entity.h"

namespace jactorio::proto
{
    class Item;

    ///
    /// Unique per entity placed in the world
    struct EntityData : FEntityData
    {
        CEREAL_SERIALIZE(archive) {
            archive(cereal::base_class<FEntityData>(this));
        }
    };

    ///
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


        /// Sprite drawn when placed in the world
        /// \remark For rotatable entities, this serves as the north sprite if multiple sprites are used
        PYTHON_PROP_I(Sprite*, sprite, nullptr);


        /// Can be rotated by player?
        PYTHON_PROP_REF_I(bool, rotatable, false);
        /// Can be placed by player?
        PYTHON_PROP_REF_I(bool, placeable, true);

        /// Seconds to pickup entity
        PYTHON_PROP_REF_I(float, pickupTime, 1);

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


        // ======================================================================
        // Renderer events

        J_NODISCARD Sprite* OnRGetSprite(SpriteSetT /*set*/) const override {
            return sprite;
        }

        J_NODISCARD SpriteSetT OnRGetSpriteSet(Orientation /*orientation*/,
                                               game::WorldData& /*world_data*/,
                                               const WorldCoord& /*world_coords*/) const override {
            return 0;
        }

        J_NODISCARD SpriteFrameT OnRGetSpriteFrame(const UniqueDataBase& /*unique_data*/,
                                                   GameTickT /*game_tick*/) const override {
            return 0;
        }

        bool OnRShowGui(const render::GuiRenderer& /*g_rendr*/, game::ChunkTileLayer* /*tile_layer*/) const override {
            return false;
        }

        // ======================================================================
        // Game events

        ///
        /// Entity was build in the world
        virtual void OnBuild(game::WorldData& world_data,
                             game::LogicData& logic_data,
                             const WorldCoord& world_coords,
                             game::ChunkTileLayer& tile_layer,
                             Orientation orientation) const = 0;

        ///
        /// Returns true if itself can be built at the specified world_coords being its top left
        /// \return true if can be built
        J_NODISCARD virtual bool OnCanBuild(const game::WorldData& /*world_data*/,
                                            const WorldCoord& /*world_coords*/) const {
            return true;
        }


        ///
        /// Entity was picked up from a built state, called BEFORE the entity has been removed
        virtual void OnRemove(game::WorldData& world_data,
                              game::LogicData& logic_data,
                              const WorldCoord& world_coords,
                              game::ChunkTileLayer& tile_layer) const = 0;

        ///
        /// A neighbor of this prototype in the world was updated
        /// \param emit_coords Coordinates of the prototype which is EMITTING the update
        /// \param receive_coords Layer of the prototype RECEIVING the update
        /// \param emit_orientation Orientation to the prototype EMITTING the update
        virtual void OnNeighborUpdate(game::WorldData& /*world_data*/,
                                      game::LogicData& /*logic_data*/,
                                      const WorldCoord& emit_coords,
                                      const WorldCoord& receive_coords,
                                      Orientation emit_orientation) const {}


        void OnDeferTimeElapsed(game::WorldData& /*world_data*/,
                                game::LogicData& /*logic_data*/,
                                UniqueDataBase* /*unique_data*/) const override {
            assert(false); // Unimplemented
        }

        void OnTileUpdate(game::WorldData& /*world_data*/,
                          const WorldCoord& /*emit_coords*/,
                          const WorldCoord& /*receive_coords*/,
                          UpdateType /*type*/) const override {
            assert(false); // Unimplemented
        }

        void OnDeserialize(game::WorldData& world_data,
                           const WorldCoord& world_coord,
                           game::ChunkTileLayer& tile_layer) const override {}


        void PostLoadValidate(const data::PrototypeManager& data_manager) const override;

    private:
        /// Item when entity is picked up
        Item* item_ = nullptr;
    };
} // namespace jactorio::proto

#endif // JACTORIO_INCLUDE_PROTO_ABSTRACT_ENTITY_H
