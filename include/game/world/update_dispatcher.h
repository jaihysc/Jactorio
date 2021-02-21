// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_WORLD_UPDATE_DISPATCHER_H
#define JACTORIO_INCLUDE_GAME_WORLD_UPDATE_DISPATCHER_H
#pragma once

#include "jactorio.h"

#include "core/coordinate_tuple.h"
#include "core/hashers.h"
#include "data/cereal/serialization_type.h"
#include "proto/framework/entity.h"

#include <cereal/types/tuple.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/vector.hpp>


namespace jactorio::game
{
    ///
    /// Calls callbacks for tile updates
    class UpdateDispatcher
    {
        using CallbackT = data::SerialProtoPtr<const proto::FEntity>;

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
        using ContainerT = std::unordered_map<ContainerKeyT, CollectionT, hash<ContainerKeyT>>;

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
        /// Registers proto_listener callback when target coord is updated, providing current coord
        ListenerEntry Register(const WorldCoord& current_coord,
                               const WorldCoord& target_coord,
                               const proto::FEntity& proto_listener);

        ///
        /// Unregister entry
        /// \return true if succeeded, false if failed
        bool Unregister(const ListenerEntry& entry);


        // World data must be provided since references cannot be serialized
        void Dispatch(World& world, const WorldCoord& coord, proto::UpdateType type);

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
