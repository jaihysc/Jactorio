// This file is subject to the terms and conditions defined in 'LICENSE' included in the source code package

#ifndef JACTORIO_INCLUDE_CORE_CONVERT_H
#define JACTORIO_INCLUDE_CORE_CONVERT_H
#pragma once

#include <type_traits>

#include "jactorio.h"

namespace jactorio::core
{
    ///
    /// Performs cast, data may be lost
    template <class TTarget, class TOriginal>
    constexpr TTarget LossyCast(
        TOriginal val,
        std::enable_if_t<std::is_integral_v<TOriginal> || std::is_floating_point_v<TOriginal>, int> = 0,
        std::enable_if_t<std::is_integral_v<TTarget> || std::is_floating_point_v<TTarget>, int>     = 0) noexcept {

        return static_cast<TTarget>(val);
    }

    ///
    /// Performs cast ensuring no data is lost
    /// \remark Same behavior as static cast if assertions are disabled
    template <class TTargetInt, class TOriginalInt>
    constexpr TTargetInt SafeCast(
        TOriginalInt val,
        std::enable_if_t<std::is_integral_v<TOriginalInt>, int>                                       = 0,
        std::enable_if_t<std::is_integral_v<TTargetInt> || std::is_floating_point_v<TTargetInt>, int> = 0) noexcept {

        constexpr bool is_different_signedness =
            (std::is_signed<TTargetInt>::value != std::is_signed<TOriginalInt>::value);

        const auto cast_val = LossyCast<TTargetInt>(val);

        if (core::LossyCast<TOriginalInt>(cast_val) != val ||
            (is_different_signedness && ((cast_val < TTargetInt{}) != (val < TOriginalInt{})))) {
            assert(false);
        }

        return cast_val;
    }

    template <typename TyLeft, typename TyRight>
    void SafeCastAssign(TyLeft& l_val, TyRight&& r_val) {
        l_val = SafeCast<std::remove_reference_t<decltype(l_val)>>(r_val);
    }

} // namespace jactorio::core

#endif // JACTORIO_INCLUDE_CORE_CONVERT_H
