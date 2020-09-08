// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_DATA_PROTOTYPE_ENTITY_DEFERRED_ENTITY_H
#define JACTORIO_DATA_PROTOTYPE_ENTITY_DEFERRED_ENTITY_H
#pragma once

namespace jactorio
{
    namespace data
    {
        struct UniqueDataBase;
    }

    namespace game
    {
        class WorldData;
        class LogicData;
    } // namespace game
} // namespace jactorio


namespace jactorio::data
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
} // namespace jactorio::data

#endif // JACTORIO_DATA_PROTOTYPE_ENTITY_DEFERRED_ENTITY_H
