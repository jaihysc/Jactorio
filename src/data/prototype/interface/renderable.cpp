// This file is subject to the terms and conditions defined in 'LICENSE' included in the source code package

#include "data/prototype/interface/renderable.h"

#include <cmath>

#include "core/convert.h"
#include "data/prototype/sprite.h"

using namespace jactorio;

SpriteFrameT data::IRenderable::AllOfSprite(Sprite& sprite, const GameTickT game_tick, const AnimationSpeed speed) {
    assert(speed > 0);

    const auto frame =
        core::LossyCast<GameTickT>(speed * game_tick) % (core::SafeCast<uint64_t>(sprite.frames) * sprite.sets);

    return core::SafeCast<SpriteFrameT>(frame);
}

SpriteFrameT data::IRenderable::AllOfSpriteReversing(Sprite& sprite,
                                                     const GameTickT game_tick,
                                                     const AnimationSpeed speed) {
    assert(speed > 0);

    // s = speed, f = frames
    // y = abs( mod(s * (x - 1/s * (f - 1)), (f * 2 - 2)) - f + 1 )

    // Graph this function to make it easier to understand

    const auto frames = core::SafeCast<uint16_t>(sprite.frames) * sprite.sets;

    // Shift the peak (which is at x = 0) such that when x = 0, y = 0
    const auto adjusted_x = game_tick - (1. / speed) * (frames - 1);

    const auto v_l = core::LossyCast<int64_t>(speed * abs(adjusted_x));
    const auto v_r = core::SafeCast<int64_t>(frames) * 2 - 2;

    const auto val = (v_l % v_r) - frames + 1;
    assert(val < frames);

    return core::SafeCast<SpriteFrameT>(abs(val));
}

SpriteFrameT data::IRenderable::AllOfSet(Sprite& sprite, const GameTickT game_tick, const AnimationSpeed speed) {
    assert(speed > 0);
    const auto frame = core::LossyCast<GameTickT>(speed * game_tick) % sprite.frames;
    return core::SafeCast<SpriteFrameT>(frame);
}
