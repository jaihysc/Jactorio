// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_GAME_LOGIC_LOGIC_DATA_H
#define JACTORIO_INCLUDE_GAME_LOGIC_LOGIC_DATA_H
#pragma once

#include "game/logic/deferral_timer.h"

namespace jactorio::game
{
    class LogicData
    {
    public:
        /// \brief Called by the logic loop every update
        void GameTickAdvance() {
            ++gameTick_;
        }

        ///
        /// \brief Number of logic updates since the world was created
        J_NODISCARD GameTickT GameTick() const {
            return gameTick_;
        }

        // ======================================================================

        ///
        /// \brief Forwards args to deferralTimer.DeferralUpdate with itself being logic data
        template <typename... TArgs>
        auto DeferralUpdate(TArgs&&... args) {
            deferralTimer.DeferralUpdate(*this, std::forward<TArgs>(args)...);
        }


        CEREAL_SERIALIZE(archive) {
            archive(deferralTimer, gameTick_);
        }

        DeferralTimer deferralTimer;

    private:
        GameTickT gameTick_ = 0;
    };
} // namespace jactorio::game

#endif // JACTORIO_INCLUDE_GAME_LOGIC_LOGIC_DATA_H
