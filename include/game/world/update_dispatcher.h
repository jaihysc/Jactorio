// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_WORLD_UPDATE_DISPATCHER_H
#define JACTORIO_INCLUDE_GAME_WORLD_UPDATE_DISPATCHER_H
#pragma once

#include "jactorio.h"

#include "core/coordinate_tuple.h"
#include "core/hashers.h"
#include "data/cereal/serialization_type.h"
#include "data/prototype/framework/entity.h"

#include <cereal/types/tuple.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/vector.hpp>


namespace jactorio::game
{
    ///
    /// Calls callbacks for tile updates
    class UpdateDispatcher
    {
        using CallbackT = data::SerialProtoPtr<const data::FEntity>;

        struct CollectionElement
        {
            WorldCoord receiver;
            CallbackT callback;

            CEREAL_SERIALIZE(archive) {
                archive(receiver, callback);
            }
        };

        using CollectionT = std::vector<CollectionElement>;

        using ContainerKeyT = std::tuple<WorldCoordAxis, WorldCoordAxis>;
        /// Emitting tile -> list of (Receiving tile + callback)
        using ContainerT = std::unordered_map<ContainerKeyT, CollectionT, core::hash<ContainerKeyT>>;

        struct DebugInfo;

    public:
        struct ListenerEntry
        {
            /// Current
            WorldCoord receiver;
            /// Registered
            WorldCoord emitter;
        };

        ///
        /// Registers proto_listener callback when target coords is updated, providing current coords
        ListenerEntry Register(WorldCoordAxis current_world_x,
                               WorldCoordAxis current_world_y,
                               WorldCoordAxis target_world_x,
                               WorldCoordAxis target_world_y,
                               const data::FEntity& proto_listener);

        ///
        /// Registers proto_listener callback when target coords is updated, providing current coords
        ListenerEntry Register(const WorldCoord& current_coords,
                               const WorldCoord& target_coords,
                               const data::FEntity& proto_listener);

        ///
        /// Unregisters entry
        /// \return true if succeeded, false if failed
        bool Unregister(const ListenerEntry& entry);


        // World data must be provided since references cannot be serialized
        void Dispatch(WorldData& world_data, WorldCoordAxis world_x, WorldCoordAxis world_y, data::UpdateType type);
        void Dispatch(WorldData& world_data, const WorldCoord& world_pair, data::UpdateType type);

        J_NODISCARD DebugInfo GetDebugInfo() const noexcept;


        CEREAL_SERIALIZE(archive) {
            archive(container_);
        }

    private:
        struct DebugInfo
        {
            const ContainerT& storedEntries;
        };

        ContainerT container_;
    };
} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_WORLD_UPDATE_DISPATCHER_H
