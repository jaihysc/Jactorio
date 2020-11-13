// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_PROTO_INTERFACE_DEFERRED_H
#define JACTORIO_INCLUDE_PROTO_INTERFACE_DEFERRED_H
#pragma once

namespace jactorio
{
    namespace proto
    {
        struct UniqueDataBase;
    }

    namespace game
    {
        class WorldData;
        class LogicData;
    } // namespace game
} // namespace jactorio


namespace jactorio::proto
{
    /// Has actions which completes at a later game tick
    class IDeferred
    {
    public:
        IDeferred()          = default;
        virtual ~IDeferred() = default;

        IDeferred(const IDeferred& other)     = default;
        IDeferred(IDeferred&& other) noexcept = default;
        IDeferred& operator=(const IDeferred& other) = default;
        IDeferred& operator=(IDeferred&& other) noexcept = default;

        ///
        /// The callback requested at the specified game tick was reached
        /// \param world_data world data containing deferred timer which dispatched this callback
        /// \param unique_data Unique data the callback was registered with
        virtual void OnDeferTimeElapsed(game::WorldData& world_data,
                                        game::LogicData& logic_data,
                                        UniqueDataBase* unique_data) const = 0;
    };
} // namespace jactorio::proto

#endif // JACTORIO_INCLUDE_PROTO_INTERFACE_DEFERRED_H
