// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_LOGIC_DEFERRAL_TIMER_H
#define JACTORIO_INCLUDE_GAME_LOGIC_DEFERRAL_TIMER_H
#pragma once

#include <unordered_map>
#include <vector>

#include "jactorio.h"

#include "core/data_type.h"
#include "data/cereal/serialization_type.h"
#include "data/cereal/serialize.h"
#include "data/prototype/framework/entity.h"

namespace jactorio::game
{
    class LogicData;

    ///
    /// Manages deferrals, prototypes inheriting 'Deferred'
    class DeferralTimer
    {
        using DeferPrototypeT  = data::FEntity;
        using DeferUniqueDataT = data::FEntityData;

        struct CallbackContainerEntry
        {
            data::SerialProtoPtr<const DeferPrototypeT> prototype;
            data::SerialUniqueDataPtr<data::UniqueDataBase> uniqueData;


            CEREAL_SERIALIZE(archive) { archive(prototype, uniqueData); }
        };

        using CallbackContainerT = std::unordered_map<GameTickT, std::vector<CallbackContainerEntry>>;

        /// 0 indicates invalid callback
        using CallbackIndex = CallbackContainerT::size_type;

        struct DebugInfo;

    public:
        ///
        /// Information about the registered deferral for removing
        struct DeferralEntry
        {
            J_NODISCARD bool Valid() const { return callbackIndex != 0; }

            void Invalidate() { callbackIndex = 0; }


            CEREAL_SERIALIZE(archive) { archive(dueTick, callbackIndex); }

            GameTickT dueTick           = 0;
            CallbackIndex callbackIndex = 0;
        };

        ///
        /// Calls all deferred callbacks for the current game tick
        /// \param game_tick Current game tick
        void DeferralUpdate(LogicData& logic_data, WorldData& world_data, GameTickT game_tick);

        ///
        /// Registers callback which will be called upon reaching the specified game tick
        /// \param deferred Implements virtual function on_defer_time_elapsed
        /// \param due_game_tick Game tick where the callback will be called
        /// \return Index of registered callback, use this to remove the callback later
        DeferralEntry RegisterAtTick(const DeferPrototypeT& deferred,
                                     DeferUniqueDataT* unique_data,
                                     GameTickT due_game_tick);

        ///
        /// Registers callback which will be called after the specified game ticks pass
        /// \param deferred Implements virtual function on_defer_time_elapsed
        /// \param elapse_game_tick Callback will be called in game ticks from now
        /// \return Index of registered callback, use this to remove the callback later
        DeferralEntry RegisterFromTick(const DeferPrototypeT& deferred,
                                       DeferUniqueDataT* unique_data,
                                       GameTickT elapse_game_tick);

        ///
        /// Removes registered callback at game_tick at index
        void RemoveDeferral(DeferralEntry entry);

        ///
        /// Removes registered callback and sets entry index to 0
        void RemoveDeferralEntry(DeferralEntry& entry);


        J_NODISCARD DebugInfo GetDebugInfo() const;


        CEREAL_SERIALIZE(archive) { archive(callbacks_, lastGameTick_); }

    private:
        CallbackContainerT callbacks_;

        GameTickT lastGameTick_ = 0;

        ///
        /// Used to fill the gap when a callback has been removed
        class BlankCallback final : public data::FEntity
        {

        public:
            PROTOTYPE_CATEGORY(none);
            void PostLoadValidate(const data::PrototypeManager& /*manager*/) const override {}

            data::Sprite* OnRGetSprite(data::Sprite::SetT /*set*/) const override { return nullptr; }

            data::Sprite::SetT OnRGetSpriteSet(data::Orientation /*orientation*/,
                                               WorldData& /*world_data*/,
                                               const WorldCoord& /*world_coords*/) const override {
                return 0;
            }

            data::Sprite::FrameT OnRGetSpriteFrame(const data::UniqueDataBase& /*unique_data*/,
                                                   GameTickT /*game_tick*/) const override {
                return 0;
            }

            bool OnRShowGui(GameWorlds& /*worlds*/,
                            game::LogicData& /*logic*/,
                            game::PlayerData& /*player*/,
                            const data::PrototypeManager& /*data_manager*/,
                            game::ChunkTileLayer* /*tile_layer*/) const override {
                return false;
            }

            void OnDeserialize(WorldData& /*world_data*/,
                               const WorldCoord& /*world_coord*/,
                               ChunkTileLayer& /*tile_layer*/) const override {}

            void OnDeferTimeElapsed(WorldData& /*world_data*/,
                                    LogicData& /*logic_data*/,
                                    data::UniqueDataBase* /*unique_data*/) const override {}

            void OnTileUpdate(WorldData& /*world_data*/,
                              const WorldCoord& /*emit_coords*/,
                              const WorldCoord& /*receive_coords*/,
                              data::UpdateType /*type*/) const override {}
        } blankCallback_;

        struct DebugInfo
        {
            const CallbackContainerT& callbacks;
        };
    };
}

#endif // JACTORIO_INCLUDE_GAME_LOGIC_DEFERRAL_TIMER_H
