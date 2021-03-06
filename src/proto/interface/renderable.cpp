// This file is subject to the terms and conditions defined in 'LICENSE' included in the source code package

#include "proto/interface/renderable.h"

#include <cmath>

#include "core/convert.h"
#include "proto/sprite.h"

using namespace jactorio;

SpriteFrameT proto::IRenderable::AllOfSprite(Sprite& sprite, const GameTickT game_tick, const AnimationSpeed speed) {
    assert(speed > 0);

    const auto frame = LossyCast<GameTickT>(speed * game_tick) % (SafeCast<uint64_t>(sprite.frames) * sprite.sets);

    return SafeCast<SpriteFrameT>(frame);
}

SpriteFrameT proto::IRenderable::AllOfSpriteReversing(Sprite& sprite,
                                                      const GameTickT game_tick,
                                                      const AnimationSpeed speed) {
    assert(speed > 0);

    // s = speed, f = frames
    // y = abs( mod(s * (x - 1/s * (f - 1)), (f * 2 - 2)) - f + 1 )

    // Graph this function to make it easier to understand

    const auto frames = SafeCast<uint16_t>(sprite.frames) * sprite.sets;

    // Shift the peak (which is at x = 0) such that when x = 0, y = 0
    const auto adjusted_x = game_tick - (1. / speed) * (frames - 1);

    const auto v_l = LossyCast<int64_t>(speed * abs(adjusted_x));
    const auto v_r = SafeCast<int64_t>(frames) * 2 - 2;

    const auto val = (v_l % v_r) - frames + 1;
    assert(val < frames);

    return SafeCast<SpriteFrameT>(abs(val));
}

SpriteFrameT proto::IRenderable::AllOfSet(Sprite& sprite, const GameTickT game_tick, const AnimationSpeed speed) {
    assert(speed > 0);
    const auto frame = LossyCast<GameTickT>(speed * game_tick) % sprite.frames;
    return SafeCast<SpriteFrameT>(frame);
}
